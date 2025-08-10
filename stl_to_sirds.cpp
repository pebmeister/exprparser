// stl_to_sirds.cpp
// compile: g++ -std=c++17 stl_to_sirds.cpp -O2 -o stl_to_sirds
// Requires stb_image.h and stb_image_write.h in the include path.




#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// include your stl header (adjust path if necessary)
#include "stl.h"

// Simple 2D/3D helpers
struct Vec3 { float x, y, z; };
struct Vec2 { int x, y; };

static inline Vec3 makev3(float x, float y, float z) { return {x,y,z}; }
static inline Vec3 sub(const Vec3 &a, const Vec3 &b){ return {a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline Vec3 cross(const Vec3 &a, const Vec3 &b){ return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }
static inline float dot(const Vec3 &a, const Vec3 &b){ return a.x*b.x + a.y*b.y + a.z*b.z; }

// Barycentric coords for point p relative to triangle a,b,c in 2D (using screen XY)
bool barycentric2D(float px, float py,
                   float ax, float ay,
                   float bx, float by,
                   float cx, float cy,
                   float &u, float &v, float &w)
{
    float denom = (by - cy)*(ax - cx) + (cx - bx)*(ay - cy);
    if (fabs(denom) < 1e-9f) return false;
    u = ((by - cy)*(px - cx) + (cx - bx)*(py - cy)) / denom;
    v = ((cy - ay)*(px - cx) + (ax - cx)*(py - cy)) / denom;
    w = 1.0f - u - v;
    return true;
}

// Rasterize triangles into a depth map (orthographic camera along -Z).
// - mesh: your stl instance (must be filled already)
// - width/height: output image size
// - scale, centerX, centerY: map mesh XY -> pixel coordinates: px = (x-centerX)*scale + width/2
// Returns depth (0..255) where 255 = nearest, 0 = far.
std::vector<uint8_t> generate_depth_map(const stl &mesh, int width, int height,
                                        float scale, float centerX, float centerY)
{
    // z-buffer initialised to +inf
    const float INF = std::numeric_limits<float>::infinity();
    std::vector<float> zbuffer(width * height, INF);
    std::vector<uint8_t> depth_map(width * height, 0);

    // Determine min/max Z in model to normalize later
    float zmin = std::numeric_limits<float>::infinity();
    float zmax = -std::numeric_limits<float>::infinity();

    size_t triCount = mesh.m_num_triangles;
    const float *vdata = mesh.m_vectors.data(); // assumed layout: tri0 v0(x,y,z), v1, v2, tri1...
    for (size_t t = 0; t < triCount; ++t) {
        // read triangle vertices
        const float *tri = vdata + t * 9;
        Vec3 v0{tri[0], tri[1], tri[2]};
        Vec3 v1{tri[3], tri[4], tri[5]};
        Vec3 v2{tri[6], tri[7], tri[8]};

        // screen coords (orthographic)
        float ax = (v0.x - centerX) * scale + width * 0.5f;
        float ay = (v0.y - centerY) * scale + height * 0.5f;
        float bx = (v1.x - centerX) * scale + width * 0.5f;
        float by = (v1.y - centerY) * scale + height * 0.5f;
        float cx = (v2.x - centerX) * scale + width * 0.5f;
        float cy = (v2.y - centerY) * scale + height * 0.5f;

        // triangle bounding box in pixel coords
        int minx = std::max(0, (int)std::floor(std::min({ax, bx, cx})));
        int maxx = std::min(width-1, (int)std::ceil(std::max({ax, bx, cx})));
        int miny = std::max(0, (int)std::floor(std::min({ay, by, cy})));
        int maxy = std::min(height-1, (int)std::ceil(std::max({ay, by, cy})));

        // pre-check degenerate
        float denom = (by - cy)*(ax - cx) + (cx - bx)*(ay - cy);
        if (fabs(denom) < 1e-9f) continue;

        // rasterize
        for (int y = miny; y <= maxy; ++y) {
            for (int x = minx; x <= maxx; ++x) {
                float u, v, w;
                if (!barycentric2D((float)x + 0.5f, (float)y + 0.5f, ax,ay, bx,by, cx,cy, u,v,w)) continue;
                // inside triangle if all barycentric >= 0 (allow small negative for edge cases)
                if (u < -1e-4f || v < -1e-4f || w < -1e-4f) continue;

                // interpolate Z from vertex Z values (v0.z, v1.z, v2.z)
                float z = u * v0.z + v * v1.z + w * v2.z;

                int idx = y * width + x;
                if (z < zbuffer[idx]) { // smaller z = closer if model uses smaller z forward; adjust if opposite.
                    zbuffer[idx] = z;
                }
            }
        }
    }

    // find finite min/max z in buffer
    for (int i = 0; i < width*height; ++i) {
        float z = zbuffer[i];
        if (std::isfinite(z)) {
            zmin = std::min(zmin, z);
            zmax = std::max(zmax, z);
        }
    }
    if (!std::isfinite(zmin) || !std::isfinite(zmax)) {
        // nothing visible: return empty map
        return depth_map;
    }
    // Normalize Z into 0..255 (we want near -> 255)
    float range = (zmax - zmin) > 1e-6f ? (zmax - zmin) : 1.0f;
    for (int i = 0; i < width*height; ++i) {
        float z = zbuffer[i];
        if (!std::isfinite(z)) {
            depth_map[i] = 0; // far
        } else {
            // map: z=zmin -> 255 (near), z=zmax -> 0 (far)
            float t = (z - zmin) / range;
            float val = (1.0f - t) * 255.0f;
            depth_map[i] = (uint8_t)std::clamp((int)std::round(val), 0, 255);
        }
    }
    return depth_map;
}

// Load texture (if not found, returns empty vector)
bool load_texture(const std::string &path, std::vector<uint8_t> &out, int &w, int &h, int &channels) {
    unsigned char *data = stbi_load(path.c_str(), &w, &h, &channels, 3);
    if (!data) return false;
    channels = 3;
    out.assign(data, data + (w * h * channels));
    stbi_image_free(data);
    return true;
}

// Create a tiled texture pixel at (x,y) by sampling the provided texture (tile) or random dots
void sample_texture_pixel(const std::vector<uint8_t> &tex, int tw, int th, int tchan,
                          std::vector<uint8_t> &outpix, int x, int y)
{
    if (tex.empty()) {
        // random dot: deterministic pseudo-noise based on y,x
        uint32_t seed = (uint32_t)((x * 73856093u) ^ (y * 19349663u));
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> d(0,255);
        uint8_t v = (uint8_t)d(rng);
        outpix[0] = v; outpix[1] = v; outpix[2] = v;
    } else {
        int sx = (x % tw + tw) % tw;
        int sy = (y % th + th) % th;
        int idx = (sy * tw + sx) * tchan;
        outpix[0] = tex[idx+0];
        outpix[1] = tex[idx+1];
        outpix[2] = tex[idx+2];
    }
}

// Generate a simple SIRDS-like image using the depth map and a tiled texture seed.
// This uses a simple left-copy chaining approach: for pixels where src_x < 0, we sample texture,
// else we copy previously computed pixels. This is simple and works for many cases;
// more advanced algorithms (union-find linking) yield crisper results for complex depth.
std::vector<uint8_t> generate_sirds(const std::vector<uint8_t> &depth, int width, int height,
                                    const std::vector<uint8_t> &tex, int tw, int th, int tchan,
                                    int pattern_width, int max_shift)
{
    const int channels = 3;
    std::vector<uint8_t> out(width * height * channels, 0);

    // initialize the first pattern_width columns by tiling the texture (or random dots)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < std::min(pattern_width, width); ++x) {
            int outidx = (y * width + x) * channels;
            uint8_t pixel[3]{0,0,0};
            sample_texture_pixel(tex, tw, th, tchan, pixel, x, y);
            out[outidx+0] = pixel[0];
            out[outidx+1] = pixel[1];
            out[outidx+2] = pixel[2];
        }
    }

    // generate remaining columns left-to-right
    for (int y = 0; y < height; ++y) {
        for (int x = pattern_width; x < width; ++x) {
            int idx = y * width + x;
            uint8_t d = depth[idx]; // 0..255
            // shift proportional to depth: deeper (d small) => larger shift (gives receding)
            // you can tweak mapping: here we use shift = (1 - d/255) * max_shift
            int shift = (int)std::round((1.0f - (d / 255.0f)) * max_shift);
            int src_x = x - pattern_width - shift;
            int outidx = idx * channels;
            if (src_x < 0) {
                // sample texture when source is outside
                uint8_t pixel[3]{0,0,0};
                sample_texture_pixel(tex, tw, th, tchan, pixel, x, y);
                out[outidx+0] = pixel[0];
                out[outidx+1] = pixel[1];
                out[outidx+2] = pixel[2];
            } else {
                int srcidx = (y * width + src_x) * channels;
                out[outidx+0] = out[srcidx+0];
                out[outidx+1] = out[srcidx+1];
                out[outidx+2] = out[srcidx+2];
            }
        }
    }
    return out;
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " input.stl [texture.png|null] out_prefix\n";
        std::cerr << "Example: ./stl_to_sirds model.stl texture.png out\n";
        std::cerr << "If texture.png is 'null', a random-dot texture will be used.\n";
        return 1;
    }
    std::string stlpath = argv[1];
    std::string texpath = argv[2];
    std::string outprefix = argv[3];

    // load mesh using your stl class
    stl mesh;
    if (mesh.read_stl(stlpath.c_str()) != 0) {
        std::cerr << "Failed to read STL: " << stlpath << "\n";
        return 1;
    }
    std::cout << "Loaded triangles: " << mesh.m_num_triangles << "\n";

    // output sizes
    const int width = 1200;
    const int height = 800;

    // compute bounding box of mesh XY/Z to pick sensible camera params
    float minx=1e9f, miny=1e9f, minz=1e9f;
    float maxx=-1e9f, maxy=-1e9f, maxz=-1e9f;
    const float *vdata = mesh.m_vectors.data();
    for (size_t i = 0; i < mesh.m_num_triangles * 9; i += 3) {
        float x = vdata[i+0], y = vdata[i+1], z = vdata[i+2];
        minx = std::min(minx, x); maxx = std::max(maxx, x);
        miny = std::min(miny, y); maxy = std::max(maxy, y);
        minz = std::min(minz, z); maxz = std::max(maxz, z);
    }
    float cx = (minx + maxx) * 0.5f;
    float cy = (miny + maxy) * 0.5f;
    float spanx = maxx - minx;
    float spany = maxy - miny;
    float span = std::max(spanx, spany);
    if (span < 1e-6f) span = 1.0f;
    // scale chosen so model fills ~80% of the dimension
    float scale = std::min(width, height) * 0.8f / span;

    std::cout << "Camera center: (" << cx << ", " << cy << ") scale=" << scale << "\n";

    // generate depth map
    auto depth = generate_depth_map(mesh, width, height, scale, cx, cy);
    // Save depth as grayscale png
    std::vector<uint8_t> depth_png(width * height * 3);
    for (int i = 0; i < width*height; ++i) {
        uint8_t v = depth[i];
        depth_png[i*3+0] = v;
        depth_png[i*3+1] = v;
        depth_png[i*3+2] = v;
    }
    std::string depth_out = outprefix + "_depth.png";
    stbi_write_png(depth_out.c_str(), width, height, 3, depth_png.data(), width*3);
    std::cout << "Wrote depth map: " << depth_out << "\n";

    // load texture if provided and not "null"
    std::vector<uint8_t> texture;
    int tw=0, th=0, tchan=0;
    bool haveTex = false;
    if (texpath != "null") {
        if (load_texture(texpath, texture, tw, th, tchan)) {
            haveTex = true;
            std::cout << "Loaded texture " << texpath << " (" << tw << "x" << th << ")\n";
        } else {
            std::cout << "Could not load texture '" << texpath << "'. Using random dots.\n";
        }
    } else {
        std::cout << "Using random dot texture.\n";
    }

    // SIRDS parameters
    int pattern_width = 80;   // seed/pattern width in pixels (try 40-150)
    int max_shift = 48;       // max horizontal shift (controls depth amplitude)
    // you can tweak these to taste

    auto sirds = generate_sirds(depth, width, height, texture, tw, th, 3, pattern_width, max_shift);
    std::string sirds_out = outprefix + "_sirds.png";
    stbi_write_png(sirds_out.c_str(), width, height, 3, sirds.data(), width*3);
    std::cout << "Wrote SIRDS: " << sirds_out << "\n";

    return 0;
}
