// stl_to_sirds_unionfind.cpp
// Single-file: requires stb_image.h and stb_image_write.h in the same dir.
// Compile: g++ -std=c++17 -O2 stl_to_sirds_unionfind.cpp -o stl_to_sirds_unionfind

#include <cmath>
#include <cstdint>
#include <cstring>
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

// ---------- Math helpers ----------
struct Vec3 { float x, y, z; };
static inline Vec3 operator-(const Vec3 &a, const Vec3 &b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline Vec3 operator+(const Vec3 &a, const Vec3 &b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline Vec3 operator*(const Vec3 &a, float s) { return {a.x*s, a.y*s, a.z*s}; }
static inline float dot(const Vec3 &a, const Vec3 &b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vec3 cross(const Vec3 &a, const Vec3 &b){ return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }
static inline float len(const Vec3 &a){ return std::sqrt(dot(a,a)); }
static inline Vec3 normalize(const Vec3 &a){ float L=len(a); return L>0 ? a*(1.0f/L) : a; }

// ---------- Camera ----------
struct Camera {
    Vec3 position;
    Vec3 look_at;
    Vec3 up;
    float fov_deg;     // used if perspective
    bool perspective;
};

// compute camera basis: right, up_cam, forward
void computeCameraBasis(const Camera &cam, Vec3 &right, Vec3 &up_cam, Vec3 &forward) {
    forward = normalize(cam.look_at - cam.position); // forward points toward scene
    right = normalize(cross(forward, cam.up));
    up_cam = cross(right, forward);
}

// project a camera-space point to NDC X,Y in [-1,1] (Z returned as camera-space z)
bool projectToNDC(const Vec3 &p_cam, float aspect, const Camera &cam, float &ndc_x, float &ndc_y, float &zcam) {
    zcam = p_cam.z; // positive = in front of camera (we require > 0)
    if (zcam <= 1e-6f) return false; // behind or too close

    if (cam.perspective) {
        // x_ndc = (x_cam / (z_cam * tan(fov/2))) / aspect? We'll scale with aspect so image doesn't distort
        float fov_rad = cam.fov_deg * (3.14159265358979323846f / 180.0f);
        float scale = std::tan(fov_rad * 0.5f); // >0
        ndc_x = (p_cam.x / (zcam * scale)) / aspect;
        ndc_y = (p_cam.y / (zcam * scale));
    } else {
        // Orthographic: assume camera-space units map directly to NDC. We will later choose a scale externally.
        ndc_x = p_cam.x;
        ndc_y = p_cam.y;
    }
    return true;
}

// ---------- Barycentric / raster ----------
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

// ---------- Depth map generation ----------
// We'll transform world vertices into camera space, project to NDC, then map to pixel coords.
// For orthographic, user-defined ortho_scale controls how many camera units map to NDC range.
std::vector<float> generate_depth_map(const stl &mesh,
                                      int width, int height,
                                      const Camera &cam,
                                      float ortho_scale /*only for ortho; NDC scale multiplier*/,
                                      float &out_zmin, float &out_zmax)
{
    // initialize z-buffer to +inf (we will store minimal zcam = distance along forward)
    const float INF = std::numeric_limits<float>::infinity();
    std::vector<float> zbuffer(width * height, INF);

    // camera basis
    Vec3 right, up_cam, forward;
    computeCameraBasis(cam, right, up_cam, forward);
    float aspect = float(width) / float(height);

    // loop triangles
    size_t triCount = mesh.m_num_triangles;
    const float *vdata = mesh.m_vectors.data(); // assumed layout: per triangle 9 floats: v0,v1,v2
    for (size_t t = 0; t < triCount; ++t) {
        const float *tri = vdata + t * 9;
        Vec3 vworld[3] = { {tri[0], tri[1], tri[2]}, {tri[3], tri[4], tri[5]}, {tri[6], tri[7], tri[8]} };
        Vec3 vcam[3];
        float ndc_x[3], ndc_y[3], zcam[3];
        bool validProj[3];

        for (int i=0;i<3;i++) {
            Vec3 rel = vworld[i] - cam.position;
            // coordinates in camera basis
            vcam[i].x = dot(rel, right);
            vcam[i].y = dot(rel, up_cam);
            vcam[i].z = dot(rel, forward); // forward axis
            Vec3 p_cam = vcam[i];
            // For orthographic we scale NDC by ortho_scale; do that by temporarily scaling p_cam.x/y
            Vec3 p_for_ndc = p_cam;
            if (!cam.perspective) {
                p_for_ndc.x = p_cam.x / ortho_scale;
                p_for_ndc.y = p_cam.y / ortho_scale;
                p_for_ndc.z = p_cam.z;
            }
            validProj[i] = projectToNDC(p_for_ndc, aspect, cam, ndc_x[i], ndc_y[i], zcam[i]);
        }

        // if all three are invalid (behind camera), skip tri
        if (!validProj[0] && !validProj[1] && !validProj[2]) continue;

        // Map NDC (-1..1) to pixel coords. If orthographic we already scaled NDC by ortho_scale above.
        float px[3], py[3];
        for (int i=0;i<3;i++) {
            // clamp ndc to -1..1 to avoid huge coordinates from perspective
            float clx = std::max(-1.0f, std::min(1.0f, ndc_x[i]));
            float cly = std::max(-1.0f, std::min(1.0f, ndc_y[i]));
            px[i] = (clx * 0.5f + 0.5f) * (width - 1);
            py[i] = ( -cly * 0.5f + 0.5f) * (height - 1); // note Y flip: NDC y up -> image y down
        }

        // bounding box in pixel coordinates
        int minx = std::max(0, (int)std::floor(std::min({px[0], px[1], px[2]})));
        int maxx = std::min(width - 1, (int)std::ceil(std::max({px[0], px[1], px[2]})));
        int miny = std::max(0, (int)std::floor(std::min({py[0], py[1], py[2]})));
        int maxy = std::min(height - 1, (int)std::ceil(std::max({py[0], py[1], py[2]})));

        // skip degenerate
        float denom = (py[1] - py[2])*(px[0] - px[2]) + (px[2] - px[1])*(py[0] - py[2]);
        if (fabs(denom) < 1e-9f) continue;

        // rasterize triangle
        for (int y = miny; y <= maxy; ++y) {
            for (int x = minx; x <= maxx; ++x) {
                float u, v, w;
                // sample at pixel center
                if (!barycentric2D((float)x + 0.5f, (float)y + 0.5f,
                                   px[0], py[0], px[1], py[1], px[2], py[2],
                                   u, v, w)) continue;
                if (u < -1e-4f || v < -1e-4f || w < -1e-4f) continue;
                // interpolate zcam (camera-space forward distance)
                // If a vertex was not projectable (behind), we still have zcam from projectToNDC for others; barycentric should still work.
                float z_interp = u * zcam[0] + v * zcam[1] + w * zcam[2];
                if (!(z_interp > 0.0f)) continue; // behind camera or invalid

                int idx = y * width + x;
                if (z_interp < zbuffer[idx]) {
                    zbuffer[idx] = z_interp;
                }
            }
        }
    }

    // find zmin/zmax among finite entries
    out_zmin = std::numeric_limits<float>::infinity();
    out_zmax = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < width * height; ++i) {
        float z = zbuffer[i];
        if (std::isfinite(z)) {
            out_zmin = std::min(out_zmin, z);
            out_zmax = std::max(out_zmax, z);
        }
    }
    // produce normalized depth map 0..1 where 1 = nearest
    std::vector<float> depth(width * height, 0.0f);
    if (!std::isfinite(out_zmin) || !std::isfinite(out_zmax)) {
        // nothing visible; return zeros
        return depth;
    }
    float range = out_zmax - out_zmin;
    if (range < 1e-6f) range = 1.0f;
    for (int i = 0; i < width * height; ++i) {
        float z = zbuffer[i];
        if (!std::isfinite(z)) depth[i] = 0.0f;
        else {
            // map: z = out_zmin (closest) -> 1.0 ; z = out_zmax (farthest) -> 0.0
            float t = (z - out_zmin) / range;
            depth[i] = 1.0f - t;
        }
    }
    return depth;
}

// ---------- Union-Find ----------
struct UnionFind {
    std::vector<int> parent;
    UnionFind(int n = 0) { parent.resize(n); for (int i = 0; i < n; ++i) parent[i] = i; }
    void reset(int n) { parent.resize(n); for (int i = 0; i < n; ++i) parent[i] = i; }
    int find(int x) { return parent[x] == x ? x : (parent[x] = find(parent[x])); }
    void unite(int a, int b) {
        a = find(a); b = find(b); if (a != b) parent[b] = a;
    }
};

// ---------- Stereogram generator (union-find), color texture aware ----------
// texture: if empty -> random-dot mode. If provided, tw/th are width/height, tchan must be 3 (RGB).
void generate_sirds_unionfind_color(const std::vector<float> &depth, int width, int height,
                                    int eye_separation, // maximum pixel separation (controls depth amplitude)
                                    const std::vector<uint8_t> &texture, int tw, int th, int tchan,
                                    std::vector<uint8_t> &out_rgb)
{
    out_rgb.assign(width * height * 3, 0);
    UnionFind uf(width);

    std::mt19937 rng(123456); // deterministic random seed for reproducibility
    std::uniform_int_distribution<int> distr(0, 255);

    for (int y = 0; y < height; ++y) {
        uf.reset(width);

        // Step 1: build unions based on depth-derived separation
        for (int x = 0; x < width; ++x) {
            float d = depth[y * width + x]; // 0..1, 1=nearest
            // map to separation (0..eye_separation)
            int sep = (int)std::round(d * eye_separation);
            // For small seps, no constraint; require sep >= 1
            if (sep < 1) continue;
            int left = x - sep/2;
            int right = left + sep;
            if (left >= 0 && right < width) uf.unite(left, right);
        }

        // Step 2: we must assign a color triplet for each root index.
        // If texture provided: sample color from texture at (root_x % tw, y % th).
        // Otherwise: generate random RGB for each root.
        std::vector<std::array<uint8_t,3>> rootColor(width);
        for (int x = 0; x < width; ++x) {
            int r = uf.find(x);
            if (r == x) {
                if (!texture.empty() && tchan >= 3 && tw>0 && th>0) {
                    int tx = (x % tw + tw) % tw;
                    int ty = (y % th + th) % th;
                    int tidx = (ty * tw + tx) * tchan;
                    uint8_t rc = texture[tidx + 0];
                    uint8_t gc = texture[tidx + 1];
                    uint8_t bc = texture[tidx + 2];
                    rootColor[x] = { rc, gc, bc };
                } else {
                    // random color
                    rootColor[x] = { (uint8_t)distr(rng), (uint8_t)distr(rng), (uint8_t)distr(rng) };
                }
            } // else we'll copy root's color later
        }

        // fill row: for each x, set color = rootColor[root]
        for (int x = 0; x < width; ++x) {
            int root = uf.find(x);
            const auto &c = rootColor[root];
            int outIdx = (y * width + x) * 3;
            out_rgb[outIdx + 0] = c[0];
            out_rgb[outIdx + 1] = c[1];
            out_rgb[outIdx + 2] = c[2];
        }
    }
}

// ---------- Helper: load texture with stb (force 3 channels RGB) ----------
bool load_texture_rgb(const std::string &path, std::vector<uint8_t> &out, int &w, int &h, int &channels) {
    int orig_ch = 0;
    unsigned char *data = stbi_load(path.c_str(), &w, &h, &orig_ch, 3);
    if (!data) return false;
    channels = 3;
    out.assign(data, data + (w * h * channels));
    stbi_image_free(data);
    return true;
}

// ---------- Main ----------
int main(int argc, char **argv)
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " input.stl texture.png/null outprefix [width height] [eye_sep] [fov_deg] [perspective]\n";
        std::cerr << "Example: ./stl_to_sirds_unionfind model.stl texture.png out 1200 800 64 45 1\n";
        std::cerr << "Use texture 'null' for random-dot.\n";
        return 1;
    }

    std::string stlpath = argv[1];
    std::string texpath = argv[2];
    std::string outprefix = argv[3];

    int width = 1200, height = 800;
    if (argc >= 6) { width = std::atoi(argv[4]); height = std::atoi(argv[5]); }
    int eye_sep = 64;
    if (argc >= 7) eye_sep = std::atoi(argv[6]);
    float fov = 45.0f;
    if (argc >= 8) fov = (float)atof(argv[7]);
    int perspective_flag = 1;
    if (argc >= 9) perspective_flag = std::atoi(argv[8]);

    // Load STL
    stl mesh;
    if (mesh.read_stl(stlpath.c_str()) != 0) {
        std::cerr << "Failed to read STL: " << stlpath << "\n";
        return 1;
    }
    std::cout << "Loaded triangles: " << mesh.m_num_triangles << "\n";

    // Compute bounding box to choose camera defaults
    float minx=1e9f, miny=1e9f, minz=1e9f;
    float maxx=-1e9f, maxy=-1e9f, maxz=-1e9f;
    const float *vdata = mesh.m_vectors.data();
    size_t vcount = mesh.m_num_triangles * 3;
    for (size_t i = 0; i < vcount; ++i) {
        float x = vdata[i*3 + 0];
        float y = vdata[i*3 + 1];
        float z = vdata[i*3 + 2];
        minx = std::min(minx, x); maxx = std::max(maxx, x);
        miny = std::min(miny, y); maxy = std::max(maxy, y);
        minz = std::min(minz, z); maxz = std::max(maxz, z);
    }
    Vec3 center = { (minx+maxx)*0.5f, (miny+maxy)*0.5f, (minz+maxz)*0.5f };
    float spanx = maxx - minx; float spany = maxy - miny; float spanz = maxz - minz;
    float span = std::max({spanx, spany, 1e-6f});

    // Setup camera: simple default that looks at center from +Z direction
    Camera cam;
    cam.look_at = center;
    cam.up = {0,1,0};
    cam.perspective = (perspective_flag != 0);
    cam.fov_deg = fov;
    // Position camera back along world Z by a distance proportional to model size.
    // We offset by spanz * 2 so all geometry is in front.
    cam.position = { center.x, center.y, center.z + span * 2.5f };

    // For orthographic we define an ortho_scale: number of world units that map to NDC extents.
    float ortho_scale = span * 0.9f; // tweak as needed

    std::cout << "Camera: pos=(" << cam.position.x << "," << cam.position.y << "," << cam.position.z
              << ") look_at=(" << cam.look_at.x << "," << cam.look_at.y << "," << cam.look_at.z
              << ") perspective=" << cam.perspective << " fov=" << cam.fov_deg << "\n";

    // Generate depth map
    float zmin, zmax;
    auto depth = generate_depth_map(mesh, width, height, cam, ortho_scale, zmin, zmax);
    std::cout << "Depth zmin=" << zmin << " zmax=" << zmax << "\n";

    // Save depth visualization image (RGB grayscale)
    std::vector<uint8_t> depth_vis(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        uint8_t v = (uint8_t)std::round(std::clamp(depth[i], 0.0f, 1.0f) * 255.0f);
        depth_vis[i*3+0] = v; depth_vis[i*3+1] = v; depth_vis[i*3+2] = v;
    }
    std::string depth_out = outprefix + "_depth.png";
    stbi_write_png(depth_out.c_str(), width, height, 3, depth_vis.data(), width * 3);
    std::cout << "Wrote depth visualization: " << depth_out << "\n";

    // Load texture if provided
    std::vector<uint8_t> texture;
    int tw=0, th=0, tchan=0;
    bool haveTex = false;
    if (texpath != "null") {
        if (load_texture_rgb(texpath, texture, tw, th, tchan)) {
            haveTex = true;
            std::cout << "Loaded texture " << texpath << " (" << tw << "x" << th << " ch=" << tchan << ")\n";
        } else {
            std::cout << "Failed to load texture '" << texpath << "'. Falling back to random dots.\n";
        }
    } else {
        std::cout << "Using random-dot texture.\n";
    }

    // Generate stereogram (union-find), tile texture or random
    std::vector<uint8_t> sirds_rgb;
    generate_sirds_unionfind_color(depth, width, height, eye_sep,
                                   texture, tw, th, tchan,
                                   sirds_rgb);

    std::string sirds_out = outprefix + "_sirds.png";
    stbi_write_png(sirds_out.c_str(), width, height, 3, sirds_rgb.data(), width * 3);
    std::cout << "Wrote stereogram: " << sirds_out << "\n";

    return 0;
}
