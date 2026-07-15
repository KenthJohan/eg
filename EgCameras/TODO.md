## Refactor camera state into more components

## Implement generic ray cast from camera

```c
static void RayCast(ecs_iter_t *it)
{
    Window *win = ecs_field(it, Window, 0);                          // shared
    EgCamerasState *cam = ecs_field(it, EgCamerasState, 1);          // self
    Position3 *pos = ecs_field(it, Position3, 2);                    // self
    Ray3 *ray1 = ecs_field(it, Ray3, 3);                             // self
    EgWindowsMouseRay *winray = ecs_field(it, EgWindowsMouseRay, 4); // self

    win->dt = it->delta_time;
    win->fps = 1.0f / it->delta_time;

    for (int i = 0; i < it->count; ++i, ++cam, ++pos, ++ray1, ++winray) {

        // TODO: What is this? Remove this:
        win->pos[0] = pos->x;
        win->pos[1] = pos->y;
        win->pos[2] = pos->z;

        // Normalize to mouse position to (-1 .. 1)
        float mouse_pos[2] = {win->canvas_mouse_x, win->canvas_mouse_y};
        float rectangle[2] = {win->canvas_width, win->canvas_height};
        float r[4];
        r[0] = 2.0f * (mouse_pos[0] / rectangle[0]) - 1.0f;
        r[1] = 2.0f * (mouse_pos[1] / rectangle[1]) - 1.0f;
        r[1] *= -1.0f; // Why flip, hmm?
        r[2] = -1.0;
        r[3] = 1.0;

        // Eye/Camera:
        float ray_eye[4];
        m4f32 pinv;
        m4f32_inverse((float *)&cam->projection, (float *)&pinv);
        m4f32_mulv(&pinv, r, ray_eye);
        ray_eye[2] = -1.0f;
        ray_eye[3] = 0.0f;

        // Convert to world coordinates:
        m4f32 vinv;
        float ray_world[4];
        m4f32_inverse((float *)&cam->view, (float *)&vinv);
        m4f32_mulv(&vinv, ray_eye, ray_world);

        ray1->x = ray_world[0];
        ray1->y = ray_world[1];
        ray1->z = ray_world[2];
    }
}
```
