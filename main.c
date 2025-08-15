#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include <stdlib.h>
#include "parson.h"
#include "linked_list.h"
#include <stdbool.h>
#include "raymath.h"
#include <time.h>
#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
#else
  #include <sys/stat.h>
  #define MKDIR(p) mkdir(p, 0755)
#endif
#include <ctype.h>

// defines
#define MAX_TEXT_LEN 16

// Typedefs
// forward function-decls
static inline Rectangle NormalizeRect(Rectangle r);
static inline Rectangle ClampRectToImage(Rectangle r, int w, int h);
static inline Rectangle ScaleRect(Rectangle r, Rectangle in, Rectangle out);
static inline void SafeUnloadImage(Image *img);
static inline void SafeUnloadTexture(Texture2D *tex);
static void CanonicalizePlate(char *s);

void freeRect(Rectangle *r)
{
    printf("freeing rect!\n");
    free(r);
}

void printRect(Rectangle r)
{
    printf("Rect: {%f,%f,%f,%f}\n", r.x, r.y, r.width, r.height);
}

int main(int argc, char *argv[], char **envp)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    Rectangle screen = {.x = 0.0f, .y = 0.0f, .width = (float)screenWidth, .height = (float)screenHeight};
    char *input_png_folder;
    size_t cur_png_idx = 0;
    List *rects;
    JSON_Value *root = NULL;
    JSON_Object *root_obj = NULL;
    JSON_Value *img_arr_val = NULL;
    JSON_Array *img_arr = NULL;
    JSON_Value *annotations = NULL;
    JSON_Array *annotation_arr = NULL;
    JSON_Value *lp_arr_val = NULL;
    JSON_Array *lp_arr = NULL;
    int annotation_id = 0;
    char status[256];
    bool saved = false;
    double saved_msg_dur = 5.0;
    double saved_msg_start = 0.0;
    bool input_mode = false;
    char inputBuffer[MAX_TEXT_LEN] = {0};
    int inputCharIndex = 0;
    size_t crop_count = 0;
    Rectangle image_bounds = {0};
    bool cropPreviewReady = false;

    if (argc < 2)
    {
        fprintf_s(stderr, "Usage: main.exe <path-to-folder-containing-pngs>\n");
        fprintf_s(stderr, "Run `ffmpeg -i <input-mp4> -vf \"select='eq(pict_type\\,I)*not(mod(n\\,4))',showinfo\" -vsync vfr <path-to-pngs>/frame_%%04d.png` to generate photos from video");
        return 1;
    }

    // parse argv[1] as the input folder of png keyframes
    input_png_folder = _strdup(argv[1]);
    if (!DirectoryExists(input_png_folder))
    {
        fprintf_s(stderr, "ERROR path %s does not exist.\n", input_png_folder);
        return 1;
    }
    FilePathList pngs = LoadDirectoryFilesEx(input_png_folder, ".png", false);
    free(input_png_folder);
    if (pngs.count <= 0)
    {
        fprintf_s(stderr, "ERROR path %s does not contains any pngs.\n", input_png_folder);
        return 1;
    }
    else
    {
        root = json_value_init_object();
        root_obj = json_value_get_object(root);
        img_arr_val = json_value_init_array();
        img_arr = json_value_get_array(img_arr_val);
        json_object_set_value(root_obj, "images", img_arr_val);

        // add the categories array
        JSON_Value *categories_arr_val = json_value_init_array();
        JSON_Array *categories_arr = json_value_get_array(categories_arr_val);
        JSON_Value *category_value = json_value_init_object();
        JSON_Object *category_obj = json_value_get_object(category_value);
        json_object_set_number(category_obj, "id", 0);
        json_object_set_string(category_obj, "name", "license_plate");
        json_array_append_value(categories_arr, category_value);
        json_object_set_value(root_obj, "categories", categories_arr_val);

        // initialize the annotations array
        annotations = json_value_init_array();
        annotation_arr = json_value_get_array(annotations);
        json_object_set_value(root_obj, "annotations", annotations);

        // initialize the lp annotations array
        lp_arr_val = json_value_init_array();
        lp_arr = json_value_get_array(lp_arr_val);
    }

    InitWindow(screenWidth, screenHeight, "RectAnnotatorAlpr Window");
    SetTargetFPS(60);

    // Make the output directories for pngs and annotation files
    MKDIR("lpcrops");
    MKDIR("lpannotations");
    MKDIR("lpannotations/86");
    MKDIR("lptextannotations");
    MKDIR("lptextannotations/86");

    Image img = LoadImage(pngs.paths[cur_png_idx]);
    Texture2D previewTexture = {0};
    Texture2D texture = LoadTextureFromImage(img);
    image_bounds.width = (float)img.width;
    image_bounds.height = (float)img.height;
    Rectangle bbox = {0};
    rects = (List *)malloc(sizeof(List));
    list_init(rects, &freeRect);
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();
        if (IsKeyPressed(KEY_S) && !input_mode)
        {
            Rectangle vis = NormalizeRect(bbox);
            crop_count++;
            // copy of the bbox and store into rects list
            Rectangle *save = (Rectangle *)malloc(sizeof(Rectangle));
            save->x = vis.x;
            save->y = vis.y;
            save->width = vis.width;
            save->height = vis.height;
            list_ins_next(rects, NULL, save);
            ListElmt *it = list_head(rects);
            while (it != NULL)
            {
                printRect(*(Rectangle *)(it->data));
                it = it->next;
            }

            // save the bbox to json mapped from screen coords to texture/image coords
            JSON_Value *cur_annotation = json_value_init_object();
            JSON_Object *cur_annot_obj = json_value_get_object(cur_annotation);
            JSON_Value *bbox_value = json_value_init_array();
            JSON_Array *bbox_arr = json_value_get_array(bbox_value);
            json_array_append_number(bbox_arr, (double)Remap(vis.x, 0.0f, (float)screenWidth, 0.0f, (float)texture.width));
            json_array_append_number(bbox_arr, (double)Remap(vis.y, 0.0f, (float)screenHeight, 0.0f, (float)texture.height));
            json_array_append_number(bbox_arr, (double)Remap(vis.width, 0.0f, (float)screenWidth, 0.0f, (float)texture.width));
            json_array_append_number(bbox_arr, (double)Remap(vis.height, 0.0f, (float)screenHeight, 0.0f, (float)texture.height));
            json_object_set_number(cur_annot_obj, "image_id", (double)cur_png_idx);
            json_object_set_number(cur_annot_obj, "category_id", 0);
            json_object_set_number(cur_annot_obj, "id", annotation_id++);
            json_object_set_value(cur_annot_obj, "bbox", bbox_value);
            json_array_append_value(annotation_arr, cur_annotation);

            // Save the image metadata to the json
            bool found = false;
            for (int saved_img_idx = 0; saved_img_idx < json_array_get_count(img_arr); saved_img_idx++)
            {
                JSON_Object *cur_img = json_array_get_object(img_arr, saved_img_idx);
                if (json_object_get_number(cur_img, "id") == cur_png_idx)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                JSON_Value *img_data = json_value_init_object();
                JSON_Object *img_obj = json_value_get_object(img_data);
                json_object_set_number(img_obj, "id", (double)cur_png_idx);
                json_object_set_string(img_obj, "file_name", GetFileName(pngs.paths[cur_png_idx]));
                json_object_set_number(img_obj, "width", img.width);
                json_object_set_number(img_obj, "height", img.height);
                json_array_append_value(img_arr, img_data);
            }
            input_mode = true;
            inputCharIndex = 0;
            memset(inputBuffer, 0, MAX_TEXT_LEN);
            while (GetCharPressed() > 0) { }
        }
        if ((IsKeyPressed(KEY_N) || IsKeyPressed(KEY_P)) && !input_mode)
        {
            cropPreviewReady = false;
            SafeUnloadTexture(&previewTexture);
            // go to next image
            cur_png_idx = IsKeyPressed(KEY_N) ? cur_png_idx + 1 : cur_png_idx - 1;
            if (cur_png_idx == pngs.count)
            {
                cur_png_idx = 0;
            }
            else if (cur_png_idx >= SIZE_MAX)
            {
                cur_png_idx = pngs.count - 1;
            }
            UnloadTexture(texture);
            UnloadImage(img);
            img = LoadImage(pngs.paths[cur_png_idx]);
            image_bounds.width = (float)img.width;
            image_bounds.height = (float)img.height;
            texture = LoadTextureFromImage(img);
            do
            {
                ListElmt *rmv;
                if (list_rem_next(rects, NULL, &rmv) == 0)
                {
                    free(rmv);
                }
            } while (list_size(rects) > 0);
            for (size_t i = 0; i < json_array_get_count(annotation_arr); ++i)
            {
                JSON_Object *tmp_annot_obj = json_array_get_object(annotation_arr, i);
                if (json_object_get_number(tmp_annot_obj, "image_id") == cur_png_idx)
                {
                    // load the stored json rectangle in texture coords to screen coords
                    Rectangle *loaded = (Rectangle *)malloc(sizeof(Rectangle));
                    JSON_Array *tmp_bbox_arr = json_object_get_array(tmp_annot_obj, "bbox");
                    for (size_t j = 0; j < json_array_get_count(tmp_bbox_arr); ++j)
                    {
                        if (j == 0)
                        {
                            loaded->x = Remap((float)json_array_get_number(tmp_bbox_arr, j), 0.0f, (float)texture.width, 0.0f, (float)screenWidth);
                        }
                        else if (j == 1)
                        {
                            loaded->y = Remap((float)json_array_get_number(tmp_bbox_arr, j), 0.0f, (float)texture.height, 0.0f, (float)screenHeight);
                        }
                        else if (j == 2)
                        {
                            loaded->width = Remap((float)json_array_get_number(tmp_bbox_arr, j), 0.0f, (float)texture.width, 0.0f, (float)screenWidth);
                        }
                        else if (j == 3)
                        {
                            loaded->height = Remap((float)json_array_get_number(tmp_bbox_arr, j), 0.0f, (float)texture.height, 0.0f, (float)screenHeight);
                        }
                    }
                    list_ins_next(rects, NULL, loaded);
                }
            }
            crop_count = list_size(rects);
        }
        if (IsKeyPressed(KEY_W) && !input_mode)
        {
            char annotationOutFile[128];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            sprintf(annotationOutFile, "lpannotations/86/%d%02d%02d%02d%02d%02d.json", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            // write the json file
            JSON_Status result = json_serialize_to_file_pretty(root, annotationOutFile);
            if (result != JSONSuccess)
            {
                fprintf(stderr, "Failed to write lpannotations file\n");
            }
            else
            {
                saved = true;
                saved_msg_start = GetTime();
            }
            sprintf(annotationOutFile, "lptextannotations/86/%d%02d%02d%02d%02d%02d.json", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            result = json_serialize_to_file_pretty(lp_arr_val, annotationOutFile);
            if (result != JSONSuccess)
            {
                fprintf(stderr, "Failed to write lptextannotations file\n");
            }
            else
            {
                saved = true;
                saved_msg_start = GetTime();
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !input_mode)
        {
            bbox.x = mousePos.x;
            bbox.y = mousePos.y;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !input_mode)
        {
            bbox.width = mousePos.x - bbox.x;
            bbox.height = mousePos.y - bbox.y;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && bbox.width > 0 && bbox.height > 0) {
            // create a zoomed-in box for license plate reading
            if (IsTextureValid(previewTexture)) {
                UnloadTexture(previewTexture);
            }
            Image cropPreview = ImageFromImage(img, ScaleRect(bbox, screen, image_bounds));
            previewTexture = LoadTextureFromImage(cropPreview);
            SafeUnloadImage(&cropPreview);
            cropPreviewReady = true;
        }

        // handle license plate input
        if (input_mode)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && inputCharIndex < MAX_TEXT_LEN - 1)
                {
                    inputBuffer[inputCharIndex++] = (char)key;
                    inputBuffer[inputCharIndex] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && inputCharIndex > 0)
            {
                inputBuffer[--inputCharIndex] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER))
            {
#ifdef _MSC_VER
_CrtCheckMemory();
#endif
                char filename[256];
                sprintf(filename, "lpcrops/crop_%03zd_%s", crop_count, GetFileName(pngs.paths[cur_png_idx]));

                // Normalize the bounding box in screen space
                Rectangle nb = NormalizeRect(bbox);
                
                // Map to image pixel coords
                Rectangle scaled = ScaleRect(nb, screen, image_bounds);
                Rectangle safe = ClampRectToImage(scaled, img.width, img.height);
                printRect(safe);
                Image cropped = ImageFromImage(img, safe);
                ImageFormat(&cropped, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                if (!ExportImage(cropped, filename)) {
                    TraceLog(LOG_ERROR, "ExportImage failed: %s", filename);
                }
                
                UnloadImage(cropped);
#ifdef _MSC_VER
_CrtCheckMemory();
#endif
                CanonicalizePlate(inputBuffer);
                JSON_Value *lp_annot_val = json_value_init_object();
                JSON_Object *lp_annot = json_value_get_object(lp_annot_val);
                json_object_set_string(lp_annot, "filename", GetFileName(filename));
                json_object_set_string(lp_annot, "text", inputBuffer);
                json_array_append_value(lp_arr, lp_annot_val);
                input_mode = false;
                bbox = (Rectangle){0};
            }

            if (IsKeyPressed(KEY_F1))
            {
                input_mode = false;
                bbox = (Rectangle){0};
            }

        }
        if (IsKeyPressed(KEY_F2) && cropPreviewReady) {
            SafeUnloadTexture(&previewTexture);
            cropPreviewReady = false;
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexturePro(texture, (Rectangle){0.0f, 0.0f, (float)texture.width, (float)texture.height}, (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight}, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        ListElmt *rect_ptr = list_head(rects);
        DrawText("[E]dit current rect. [S]ave current rect. [W]rite JSON annotations.json. [N]ext image.", 10, 10, 15, GREEN);
        sprintf_s(status, sizeof(status), "Image %zu of %u\n (%s)", cur_png_idx + 1, pngs.count, GetFileName(pngs.paths[cur_png_idx]));
        DrawText(status, 10, 30, 15, GREEN);
        if (saved)
        {
            DrawText("File annotations.json saved!", 10, 50, 15, GREEN);
            if (saved_msg_start + saved_msg_dur < GetTime())
            {
                saved = false;
            }
        }
        while (rect_ptr != NULL)
        {
            Rectangle q = *(Rectangle *)rect_ptr->data;
            DrawRectangleLinesEx(q, 1.0f, GREEN);
            rect_ptr = rect_ptr->next;
        }
        if (input_mode)
        {
            DrawRectangle(100, 100, 300, 60, LIGHTGRAY);
            DrawText("Enter plate text:", 110, 110, 20, DARKGRAY);
            DrawText(inputBuffer, 110, 135, 24, BLACK);
            DrawText("Press ENTER to save, F1 to skip", 100, 170, 14, DARKGRAY);
        }

        if (cropPreviewReady) {
            Rectangle previewSrc = (Rectangle) {
                .x = 0.0f,
                .y = 0.0f,
                .width = (float)previewTexture.width,
                .height = (float)previewTexture.height,
            };
            Rectangle previewDest = (Rectangle) {
                .x = (float)(screenWidth - (previewTexture.width * 2)),
                .y = 0.0f,
                .width = (float)(previewTexture.width * 2),
                .height = (float)(previewTexture.height * 2),
            };
            DrawTexturePro(previewTexture, previewSrc, previewDest, (Vector2){0}, 0.0f, WHITE);
            DrawText("F2 to close preview", screenWidth - (previewTexture.width * 2), 0, 12, GREEN);
        }

        // Draw the current Rectangle
        DrawRectangleLinesEx(bbox, 1.0f, RED);
        EndDrawing();
    }
    CloseWindow();
    list_destroy(rects);
    json_value_free(root);
    return 0;
}

static inline Rectangle ScaleRect(Rectangle nb, Rectangle in, Rectangle out) {
    return (Rectangle){
        Remap(nb.x, in.x, in.width, out.x, out.width),
        Remap(nb.y, in.y, in.height, out.y, out.height),
        Remap(nb.width, in.x, in.width, out.x, out.width),
        Remap(nb.height, in.y, in.height, out.y, out.height),
    };
}

static inline Rectangle NormalizeRect(Rectangle r) {
    if (r.width < 0)  { r.x += r.width;  r.width  = -r.width; }
    if (r.height < 0) { r.y += r.height; r.height = -r.height; }
    return r;
}

static inline Rectangle ClampRectToImage(Rectangle r, int w, int h) {
    r = NormalizeRect(r);
    float x0 = (r.x < 0) ? 0 : r.x;
    float y0 = (r.y < 0) ? 0 : r.y;
    float x1 = (r.x + r.width  > w) ? (float)w : (r.x + r.width);
    float y1 = (r.y + r.height > h) ? (float)h : (r.y + r.height);
    int ix0 = (int)floorf(x0), iy0 = (int)floorf(y0);
    int ix1 = (int)ceilf(x1),  iy1 = (int)ceilf(y1);
    if (ix1 <= ix0) ix1 = ix0 + 1;
    if (iy1 <= iy0) iy1 = iy0 + 1;
    if (ix1 > w) ix1 = w;
    if (iy1 > h) iy1 = h;
    return (Rectangle){ (float)ix0, (float)iy0, (float)(ix1 - ix0), (float)(iy1 - iy0) };
}

static void CanonicalizePlate(char *s) {
    // Uppercase and strip spaces/dashes (optional)
    char *src = s, *dst = s;
    while (*src) {
        char c = (char)toupper((unsigned char)*src++);
        if (c == ' ' || c == '-') continue;     // keep if you want, or drop
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) *dst++ = c;
        // else: skip any unsupported char
    }
    *dst = '\0';
}

static inline void SafeUnloadImage(Image *img) {
    if (img && img->data) { UnloadImage(*img); *img = (Image){0}; }
}

static inline void SafeUnloadTexture(Texture2D *tex) {
    if (tex && tex->id != 0) { UnloadTexture(*tex); *tex = (Texture2D){0}; }
}