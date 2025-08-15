#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include "parson.h"
#include <stdlib.h>
#include <time.h>
#include "linked_list.h"
// cl test_main.c parson.c linked_list.c ^
// -I build\vcpkg_installed\x64-windows\include ^
// /link /LIBPATH:build\vcpkg_installed\x64-windows\lib raylib.lib glfw3dll.lib nanosvg.lib nanosvgrast.lib

void destroyRect(Rectangle *r) {
    printf("destroying rect!");
    free(r);
}

void printRect(Rectangle r) {
    printf("Rect: {%f,%f,%f,%f}\n", r.x, r.y, r.width, r.height);
}

int main(void) {
    printf("Hello, World!\n");
    int screenWidth = 1280, screenHeight = 720;
    srand(time(NULL));
    JSON_Value *r = json_parse_file("./license-plate-detection/dataset/annotations.json");
    printf("Finished parsing annotations file %s\n", r == NULL ? "NULL" : "SUCCESS");
    JSON_Object *ro = json_value_get_object(r);
    JSON_Value *i = json_object_get_value(ro, "images");
    JSON_Array *io = json_value_get_array(i);
    JSON_Value *a = json_object_get_value(ro, "annotations");
    JSON_Array *ao = json_value_get_array(a);
    size_t random_idx = rand() % json_array_get_count(io);
    printf("size of images array %zu\n", json_array_get_count(io));
    JSON_Object *rx = json_array_get_object(io, random_idx);

    printf("Random image idx = %d\n", random_idx);
    const char image_file_path[128];
    sprintf_s(image_file_path, sizeof(image_file_path), "%s/%s", "license-plate-detection/dataset/images", json_object_get_string(rx, "file_name"));

    printf("Random file_path = %s\n", image_file_path);
    List *annotations = (List *)malloc(sizeof(List));
    list_init(annotations, &destroyRect);

    for (size_t i = 0; i < json_array_get_count(ao); ++i) {
        JSON_Object *o = json_array_get_object(ao, i);
        if (json_object_get_number(o, "image_id") == json_object_get_number(rx, "id")) {
            Rectangle *rp = (Rectangle *)malloc(sizeof(Rectangle));
            JSON_Value *bboxv = json_object_get_value(o, "bbox");
            JSON_Array *bboxa = json_value_get_array(bboxv); 
            for (size_t j = 0; j < json_array_get_count(bboxa); ++j) {
                if (j == 0) {
                    rp->x = json_array_get_number(bboxa, j);
                } else if (j == 1) {
                    rp->y = json_array_get_number(bboxa, j);
                } else if (j == 2) {
                    rp->width = json_array_get_number(bboxa, j);
                } else if (j == 3) {
                    rp->height = json_array_get_number(bboxa, j);
                }
            }
            printRect(*rp);
            list_ins_next(annotations, NULL, rp);
        }
    }
    printf("INIT WINDOW BEGIN\n");
    InitWindow(1280, 720, "validate bboxes");
    printf("INIT WINDOW END\n");
    SetTargetFPS(60);
    Image img = LoadImage(image_file_path);
    Texture2D texture = LoadTextureFromImage(img);
    Rectangle source = {.x = 0.0f, .y = 0.0f, .width = (float)texture.width, .height = (float)texture.height};
    Rectangle destination = {.x = 0.0f, .y = 0.0f, .width = (float)screenWidth, .height = (float)screenHeight};
    Vector2 origin = {.x = 0.0f, .y = 0.0f};
    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawTexturePro(texture, source, destination, origin, 0.0f, WHITE);
        ListElmt *c = list_head(annotations);
        while(c != NULL) {
            Rectangle rt = *(Rectangle *)(c->data);
            rt.x = Remap(rt.x, source.x, source.width, destination.x, destination.width);
            rt.y = Remap(rt.y, source.y, source.height, destination.y, destination.height);
            rt.width = Remap(rt.width, source.x, source.width, destination.x, destination.width);
            rt.height = Remap(rt.height, source.y, source.height, destination.y, destination.height);
            DrawRectangleLinesEx(rt, 1.0f, GREEN);
            c = c->next;
        }
        EndDrawing();
    }
    CloseWindow();
    return;
}