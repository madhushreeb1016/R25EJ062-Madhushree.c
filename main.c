#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#define HEIGHT 25
#define WIDTH 80
#define MAX_SHAPES 100
#define BG_CHAR '_'
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;
typedef struct {
    int x1, y1, x2, y2;
} LineParams;
typedef struct {
    int x, y, w, h;
} RectParams;
typedef struct {
    int cx, cy, r;
} CircleParams;
typedef struct {
    int x1, y1, x2, y2, x3, y3;
} TriParams;
typedef struct {
    int id;
    ShapeType type;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriParams tri;
    } data;
    char ch;
    bool active;
} Shape;
// Global State
Shape shapes[MAX_SHAPES];
int next_id = 1;
char canvas[HEIGHT][WIDTH];
// Utility: Clear console screen
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
// Utility: Clear standard input buffer
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
// Canvas Functions
void clear_canvas() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = BG_CHAR;
        }
    }
}
void display_canvas() {
    // Top border
    for (int x = 0; x < WIDTH + 2; x++) printf("=");
    printf("\n");
    for (int y = 0; y < HEIGHT; y++) {
        printf("|"); // Left border
        for (int x = 0; x < WIDTH; x++) {
            putchar(canvas[y][x]);
        }
        printf("|\n"); // Right border
    }
    // Bottom border
    for (int x = 0; x < WIDTH + 2; x++) printf("=");
    printf("\n");
}
// Drawing Algorithms
void draw_line(int x1, int y1, int x2, int y2, char ch) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        if (x1 >= 0 && x1 < WIDTH && y1 >= 0 && y1 < HEIGHT) {
            canvas[y1][x1] = ch;
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
void draw_rectangle(int x, int y, int w, int h, char ch) {
    // Draw top and bottom sides
    for (int col = x; col < x + w; col++) {
        if (col >= 0 && col < WIDTH) {
            if (y >= 0 && y < HEIGHT) canvas[y][col] = ch;
            if (y + h - 1 >= 0 && y + h - 1 < HEIGHT) canvas[y + h - 1][col] = ch;
        }
    }
    // Draw left and right sides
    for (int row = y; row < y + h; row++) {
        if (row >= 0 && row < HEIGHT) {
            if (x >= 0 && x < WIDTH) canvas[row][x] = ch;
            if (x + w - 1 >= 0 && x + w - 1 < WIDTH) canvas[row][x + w - 1] = ch;
        }
    }
}
void draw_circle_points(int cx, int cy, int x, int y, char ch) {
    int px[8] = {cx + x, cx - x, cx + x, cx - x, cx + y, cx - y, cx + y, cx - y};
    int py[8] = {cy + y, cy + y, cy - y, cy - y, cy + x, cy + x, cy - x, cy - x};
    for (int i = 0; i < 8; i++) {
        if (px[i] >= 0 && px[i] < WIDTH && py[i] >= 0 && py[i] < HEIGHT) {
            canvas[py[i]][px[i]] = ch;
        }
    }
}
void draw_circle(int cx, int cy, int r, char ch) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    draw_circle_points(cx, cy, x, y, ch);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(cx, cy, x, y, ch);
    }
}
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    draw_line(x1, y1, x2, y2, ch);
    draw_line(x2, y2, x3, y3, ch);
    draw_line(x3, y3, x1, y1, ch);
}
// Render all active shapes to the canvas
void render_all_shapes() {
    clear_canvas();
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    draw_line(shapes[i].data.line.x1, shapes[i].data.line.y1,
                              shapes[i].data.line.x2, shapes[i].data.line.y2,
                              shapes[i].ch);
                    break;
                case SHAPE_RECTANGLE:
                    draw_rectangle(shapes[i].data.rect.x, shapes[i].data.rect.y,
                                   shapes[i].data.rect.w, shapes[i].data.rect.h,
                                   shapes[i].ch);
                    break;
                case SHAPE_CIRCLE:
                    draw_circle(shapes[i].data.circle.cx, shapes[i].data.circle.cy,
                                shapes[i].data.circle.r, shapes[i].ch);
                    break;
                case SHAPE_TRIANGLE:
                    draw_triangle(shapes[i].data.tri.x1, shapes[i].data.tri.y1,
                                  shapes[i].data.tri.x2, shapes[i].data.tri.y2,
                                  shapes[i].data.tri.x3, shapes[i].data.tri.y3,
                                  shapes[i].ch);
                    break;
            }
        }
    }
}
// Shape management
void list_shapes() {
    printf("\n--- Active Shapes ---\n");
    int count = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            count++;
            printf("[ID: %d] ", shapes[i].id);
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    printf("LINE: from (%d, %d) to (%d, %d) using '%c'\n",
                           shapes[i].data.line.x1, shapes[i].data.line.y1,
                           shapes[i].data.line.x2, shapes[i].data.line.y2,
                           shapes[i].ch);
                    break;
                case SHAPE_RECTANGLE:
                    printf("RECTANGLE: top-left (%d, %d), size %dx%d using '%c'\n",
                           shapes[i].data.rect.x, shapes[i].data.rect.y,
                           shapes[i].data.rect.w, shapes[i].data.rect.h,
                           shapes[i].ch);
                    break;
                case SHAPE_CIRCLE:
                    printf("CIRCLE: center (%d, %d), radius %d using '%c'\n",
                           shapes[i].data.circle.cx, shapes[i].data.circle.cy,
                           shapes[i].data.circle.r, shapes[i].ch);
                    break;
                case SHAPE_TRIANGLE:
                    printf("TRIANGLE: vertices (%d, %d), (%d, %d), (%d, %d) using '%c'\n",
                           shapes[i].data.tri.x1, shapes[i].data.tri.y1,
                           shapes[i].data.tri.x2, shapes[i].data.tri.y2,
                           shapes[i].data.tri.x3, shapes[i].data.tri.y3,
                           shapes[i].ch);
                    break;
            }
        }
    }
    if (count == 0) {
        printf("(No shapes added yet)\n");
    }
    printf("---------------------\n");
}
int find_shape_index(int id) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            return i;
        }
    }
    return -1;
}
void prompt_line_params(LineParams *params) {
    printf("Enter x1 y1 (start point, e.g. 10 5): ");
    scanf("%d %d", &params->x1, &params->y1);
    printf("Enter x2 y2 (end point, e.g. 30 15): ");
    scanf("%d %d", &params->x2, &params->y2);
}
void prompt_rect_params(RectParams *params) {
    printf("Enter x y (top-left corner, e.g. 5 5): ");
    scanf("%d %d", &params->x, &params->y);
    printf("Enter width height (e.g. 15 8): ");
    scanf("%d %d", &params->w, &params->h);
}
void prompt_circle_params(CircleParams *params) {
    printf("Enter cx cy (center point, e.g. 40 12): ");
    scanf("%d %d", &params->cx, &params->cy);
    printf("Enter radius (e.g. 6): ");
    scanf("%d", &params->r);
}
void prompt_tri_params(TriParams *params) {
    printf("Enter x1 y1 (first vertex, e.g. 20 5): ");
    scanf("%d %d", &params->x1, &params->y1);
    printf("Enter x2 y2 (second vertex, e.g. 10 15): ");
    scanf("%d %d", &params->x2, &params->y2);
    printf("Enter x3 y3 (third vertex, e.g. 30 15): ");
    scanf("%d %d", &params->x3, &params->y3);
}
void action_add_shape() {
    int slot = -1;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        printf("Error: Maximum shapes reached (%d).\n", MAX_SHAPES);
        printf("Press Enter to continue...");
        clear_input_buffer();
        getchar();
        return;
    }
    printf("\nChoose Shape Type:\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("Selection: ");
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
        printf("Invalid selection.\n");
        clear_input_buffer();
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    shapes[slot].type = (ShapeType)choice;
    switch (shapes[slot].type) {
        case SHAPE_LINE:
            prompt_line_params(&shapes[slot].data.line);
            break;
        case SHAPE_RECTANGLE:
            prompt_rect_params(&shapes[slot].data.rect);
            break;
        case SHAPE_CIRCLE:
            prompt_circle_params(&shapes[slot].data.circle);
            break;
        case SHAPE_TRIANGLE:
            prompt_tri_params(&shapes[slot].data.tri);
            break;
    }
    printf("Enter drawing character (default '*'): ");
    clear_input_buffer();
    char ch = getchar();
    if (ch == '\n' || ch == EOF) {
        shapes[slot].ch = '*';
    } else {
        shapes[slot].ch = ch;
        clear_input_buffer(); // clear potential extra characters
    }
    shapes[slot].id = next_id++;
    shapes[slot].active = true;
}
void action_delete_shape() {
    printf("\nEnter ID of the shape to delete: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clear_input_buffer();
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    int idx = find_shape_index(id);
    if (idx == -1) {
        printf("Shape ID %d not found.\n", id);
        clear_input_buffer();
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    shapes[idx].active = false;
    printf("Shape ID %d deleted successfully.\n", id);
    clear_input_buffer();
    printf("Press Enter to continue...");
    getchar();
}
void action_modify_shape() {
    printf("\nEnter ID of the shape to modify: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clear_input_buffer();
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    int idx = find_shape_index(id);
    if (idx == -1) {
        printf("Shape ID %d not found.\n", id);
        clear_input_buffer();
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    printf("\nModifying Shape ID %d...\n", id);
    switch (shapes[idx].type) {
        case SHAPE_LINE:
            prompt_line_params(&shapes[idx].data.line);
            break;
        case SHAPE_RECTANGLE:
            prompt_rect_params(&shapes[idx].data.rect);
            break;
        case SHAPE_CIRCLE:
            prompt_circle_params(&shapes[idx].data.circle);
            break;
        case SHAPE_TRIANGLE:
            prompt_tri_params(&shapes[idx].data.tri);
            break;
    }
    printf("Enter drawing character (current '%c', press Enter to keep): ", shapes[idx].ch);
    clear_input_buffer();
    char ch = getchar();
    if (ch != '\n' && ch != EOF) {
        shapes[idx].ch = ch;
        clear_input_buffer(); // clear potential extra characters
    }
}
int main() {
    // Initialize shapes array
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = false;
    }
    // Add some default shapes to start with a nice demonstration
    // Shape 1: A rectangle outline
    shapes[0].id = next_id++;
    shapes[0].type = SHAPE_RECTANGLE;
    shapes[0].data.rect.x = 2;
    shapes[0].data.rect.y = 2;
    shapes[0].data.rect.w = 15;
    shapes[0].data.rect.h = 8;
    shapes[0].ch = '*';
    shapes[0].active = true;
    // Shape 2: A circle
    shapes[1].id = next_id++;
    shapes[1].type = SHAPE_CIRCLE;
    shapes[1].data.circle.cx = 45;
    shapes[1].data.circle.cy = 10;
    shapes[1].data.circle.r = 6;
    shapes[1].ch = 'O';
    shapes[1].active = true;
    // Shape 3: A line crossing the center
    shapes[2].id = next_id++;
    shapes[2].type = SHAPE_LINE;
    shapes[2].data.line.x1 = 20;
    shapes[2].data.line.y1 = 20;
    shapes[2].data.line.x2 = 70;
    shapes[2].data.line.y2 = 5;
    shapes[2].ch = '+';
    shapes[2].active = true;
    // Shape 4: A triangle
    shapes[3].id = next_id++;
    shapes[3].type = SHAPE_TRIANGLE;
    shapes[3].data.tri.x1 = 10;
    shapes[3].data.tri.y1 = 22;
    shapes[3].data.tri.x2 = 25;
    shapes[3].data.tri.y2 = 12;
    shapes[3].data.tri.x3 = 30;
    shapes[3].data.tri.y3 = 22;
    shapes[3].ch = '^';
    shapes[3].active = true;
    char option = ' ';
    while (option != 'q' && option != 'Q') {
        render_all_shapes();
        clear_screen();
        printf("======================= 2D C GRAPHICS EDITOR =======================\n");
        display_canvas();
        list_shapes();
        printf("\nOptions: [A] Add Shape  [D] Delete Shape  [M] Modify Shape  [Q] Quit\n");
        printf("Select option: ");
        
        if (scanf(" %c", &option) != 1) {
            clear_input_buffer();
            continue;
        }
        switch (option) {
            case 'a':
            case 'A':
                action_add_shape();
                break;
            case 'd':
            case 'D':
                action_delete_shape();
                break;
            case 'm':
            case 'M':
                action_modify_shape();
                break;
            case 'q':
            case 'Q':
                printf("\nExiting editor. Goodbye!\n");
                break;
            default:
                printf("Invalid option. Press Enter to continue...");
                clear_input_buffer();
                getchar();
                break;
        }
    }
    return 0;
}
