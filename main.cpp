#include <iostream>

#define SDL_MAIN_HANDLED
#include "mgui.cpp"

#define WIDTH 160
#define HEIGHT 120

// ray-triangle intersection
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution.html

// generating camera rays
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html

// 4x4 * 4x1 matrix mult
// https://global.discourse-cdn.com/sketchup/original/2X/3/372568218b9d69fdfbbef21266344cf61c2d3a01.png

void set_buffer(uint32_t* buffer, int r, int g, int b) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i] = 0xFF << 24 | r << 16 | g << 8 | b;
    }
}

struct Triangle {
    int a, b, c;
    unsigned char c_r, c_g, c_b;
};

struct Transform {
    Vec3 pos;
    Vec3 rot;
};

struct Mesh {
    Vec3* verts;
    int vert_c;
    Triangle* tris;
    int tri_c;
    Transform t;
};

struct Ray {
    Vec3 origin, direction;
};

Vec3 plane_normal(Vec3 a, Vec3 b, Vec3 c) {
    Vec3 e0 = b - a;
    Vec3 e1 = c - a;
    Vec3 N = cross_Vec3(e0, e1);
    return normalize_Vec3(N);
}

struct Ray_Hit {
    bool hit;
    float dist;
};

Ray_Hit ray_intersects_triangle(Ray* r, Vec3* a, Vec3* b, Vec3* c) {
    Vec3 tri_norm = plane_normal(*a, *b, *c);
    if (dot_Vec3(tri_norm, r->direction) == 0) {
        printf("1 ");
        return {false, 0};
    }
    float d = dot_Vec3(tri_norm, *a);
    float t = -(dot_Vec3(tri_norm, r->origin) + d) / dot_Vec3(tri_norm, r->direction);
    if (t < 0) {
        printf("2 ");
        return {false, 0};
    }
    Vec3 hit = r->origin + (r->direction * t);
    Vec3 perpendicular_vec = cross_Vec3(*b - *a, hit - *a);
    if (dot_Vec3(tri_norm, perpendicular_vec) < 0) {
        printf("3 ");
        return {false, 0};
    }
    perpendicular_vec = cross_Vec3(*c - *b, hit - *b);
    if (dot_Vec3(tri_norm, perpendicular_vec) < 0) {
        printf("4 ");
        return {false, 0};
    }
    perpendicular_vec = cross_Vec3(*a - *c, hit - *c);
    if (dot_Vec3(tri_norm, perpendicular_vec) < 0) {
        printf("5 ");
        return {false, 0};
    }
    printf("6 ");
    return {true, sqrt(powf(hit.x - r->origin.x, 2) + powf(hit.y - r->origin.y, 2) + powf(hit.z - r->origin.z, 2))};
}

void rot_mat(Mat3x3* m, Vec3* r) {
    float cos_x = cos(r->x);
    float cos_y = cos(r->y);
    float cos_z = cos(r->z);
    float sin_x = sin(r->x);
    float sin_y = sin(r->y);
    float sin_z = sin(r->z);
    m->a = cos_x * cos_y;
    m->b = cos_x * sin_y * sin_z - sin_x * cos_z;
    m->c = cos_x * sin_y * cos_z + sin_x * sin_z;
    m->d = sin_x * cos_y;
    m->e = sin_x * sin_y * sin_z + cos_x * cos_z;
    m->f = sin_x * sin_y * cos_z - cos_x * sin_z;
    m->g = -sin_y;
    m->h = cos_y * sin_z;
    m->i = cos_y * cos_z;
}

void render(uint32_t* buffer, float fov, float vp_w, float vp_h, Mesh* meshes, int meshes_c, Vec3 c_pos, Vec3 c_rot) {
    Mat4x4 c_to_w = {cos(c_rot.x)*cos(c_rot.y),cos(c_rot.x)*sin(c_rot.y)*sin(c_rot.z)-sin(c_rot.x)*cos(c_rot.z),cos(c_rot.x)*sin(c_rot.y)*cos(c_rot.z)+sin(c_rot.x)*sin(c_rot.z),c_pos.x,sin(c_rot.x)*sin(c_rot.y),sin(c_rot.x)*sin(c_rot.y)*sin(c_rot.z)+cos(c_rot.x)*cos(c_rot.z),sin(c_rot.x)*sin(c_rot.y)*cos(c_rot.z)-cos(c_rot.x)*sin(c_rot.z),c_pos.y,-sin(c_rot.y),cos(c_rot.y)*sin(c_rot.z),cos(c_rot.y)*cos(c_rot.z),c_pos.z,0,0,0,1};
    uint32_t* pixel = buffer;
    float scale = tan(rads(fov * 0.5));
    float aspect = WIDTH / HEIGHT;
    Ray_Hit hit;
    Ray_Hit closest_hit = {false, 0};
    Triangle* hit_tri = nullptr;
    Vec3 tri_a;
    Vec3 tri_b;
    Vec3 tri_c;
    Mat3x3 r_mat;
    for (uint32_t i = 0; i < WIDTH; i++) {
        for (uint32_t j = 0; j < HEIGHT; j++) {
            float x = (2 * (i + 0.5) / (float)WIDTH - 1) * aspect * scale;
            float y = (1 - 2 * (j + 0.5) / (float)HEIGHT) * scale;
            Ray r = {c_pos, mult_Mat4x4_Vec3(c_to_w, {x, y, 1})}; 
            r.direction = normalize_Vec3(r.direction);
            printf("%f, %f, %f\t%f, %f, %f\t", r.origin.x, r.origin.y, r.origin.z, r.direction.x, r.direction.y, r.direction.z);
            for (int mesh = 0; mesh < meshes_c; mesh++) {
                for (int tri = 0; tri < meshes[mesh].tri_c; tri++) {
                    rot_mat(&r_mat, &meshes[mesh].t.rot);
                    tri_a = mult_Mat3x3_Vec3(r_mat, meshes[mesh].verts[meshes[mesh].tris[tri].a]) + meshes[mesh].t.pos;
                    tri_b = mult_Mat3x3_Vec3(r_mat, meshes[mesh].verts[meshes[mesh].tris[tri].b]) + meshes[mesh].t.pos;
                    tri_c = mult_Mat3x3_Vec3(r_mat, meshes[mesh].verts[meshes[mesh].tris[tri].c]) + meshes[mesh].t.pos;
                    hit = ray_intersects_triangle(&r, &tri_a, &tri_b, &tri_c);
                    if (hit.hit && hit.dist < closest_hit.dist) {
                        closest_hit = hit;
                        hit_tri = &meshes[mesh].tris[tri];
                    }
                }
            }
            printf("%s\n", hit_tri != nullptr ? "triangle" : "nullptr");
            if (hit_tri != nullptr) {
                *(pixel++) = 255 << 24 | hit_tri->c_r << 16 | hit_tri->c_g << 8 | hit_tri->c_b;
            }
        }
    }
}

int main() {
    State state = State("test", WIDTH, HEIGHT, 120, 0);
    state.r.init_frame_buffer();
    uint32_t* buffer = (uint32_t*)malloc(state.r.w * state.r.h * sizeof(uint32_t));
    
    // Vec3* verts = (Vec3*)malloc(sizeof(Vec3) * 8);
    // verts[0] = {0.5, 0.5, 0.5};
    // verts[1] = {0.5, 0.5, -0.5};
    // verts[2] = {0.5, -0.5, 0.5};
    // verts[3] = {0.5, -0.5, -0.5};
    // verts[4] = {-0.5, 0.5, 0.5};
    // verts[5] = {-0.5, 0.5, -0.5};
    // verts[6] = {-0.5, -0.5, 0.5};
    // verts[7] = {-0.5, -0.5, -0.5};
    // Triangle* tris = (Triangle*)malloc(sizeof(Triangle) * 12);
    // tris[ 0] = {2, 1, 0, 0, 0, 127};
    // tris[ 1] = {3, 2, 1, 0, 0, 255};
    // tris[ 2] = {6, 4, 0, 0, 127, 0};
    // tris[ 3] = {6, 2, 0, 0, 127, 127};
    // tris[ 4] = {7, 5, 1, 0, 127, 255};
    // tris[ 5] = {7, 3, 1, 0, 255, 0};
    // tris[ 6] = {7, 6, 2, 0, 255, 127};
    // tris[ 7] = {7, 3, 2, 0, 255, 255};
    // tris[ 8] = {5, 4, 0, 127, 0, 0};
    // tris[ 9] = {5, 1, 0, 127, 0, 127};
    // tris[10] = {7, 6, 4, 127, 0, 255};
    // tris[11] = {7, 5, 4, 127, 127, 0};
    // Mesh cube = {verts, 8, tris, 12, {{0, 0, 0}, {rads(45), 0, 0}}};

    Vec3* verts = (Vec3*)malloc(sizeof(Vec3) * 3);
    verts[0] = {0, 1, 0};
    verts[1] = {0, 0, 0};
    verts[2] = {1, 0, 0};
    Triangle* tris = (Triangle*)malloc(sizeof(Triangle));
    tris[0] = {2, 1, 0, 255, 255, 255};
    Mesh triangle = {verts, 3, tris, 1, {{0, 0, 0}, {0, 0, 0}}};

    const float vp_w = WIDTH / 4;
    const float vp_h = HEIGHT / 4;

    while (!state.quit) {
        state.start_frame();

        memset(buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
        
        render(buffer, 70, vp_w, vp_h, &triangle, 1, {0, 0, -2}, {0, 0, 0});

        // cube.t.rot.y += rads(15);
        // if (cube.t.rot.y >= rads(360)) {
        //     cube.t.rot.y = 0;
        // }
        // cube.t.pos.x += 0.1;
        // if (cube.t.pos.x > 5) {
        //     cube.t.pos.x = -5;
        // }

        printf("fps: %f\tdt: %f\n", state.t.fps, state.t.dt);

        state.r.set_frame_buffer(buffer);
        state.end_frame();
    }
    free(buffer);
    state.r.destroy_frame_buffer();
}