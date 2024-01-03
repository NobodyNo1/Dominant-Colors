#define SUPPORT_FILEFORMAT_JPG 1
#include "raylib.h"
#include "dominant_color.h"
#include "stdbool.h"

#define DEFAULT_IMAGE_PATH  "res/crab.png"
#define DEFAULT_K           5
#define MAX_FILEPATH_SIZE   2048

void GetDominantColors(
    char* image_path,
    int image_size, 
    Color** resultColor, 
    Texture2D* texture, 
    int k,
    float* scale
) {
    //TODO: Check if path is images
    RgbColor* colors;
    // what is channel?
    // const int channelCount = 3;
    domCol(image_path, &colors, k);
    // Texture2D texture = LoadTextureFromImage(img); 
   
    for(int i = 0; i<k;i++){
        Color tmp = {
            .r = colors[i].r, .g = colors[i].g, .b = colors[i].b , .a = 255
        };
        (*resultColor)[i] = tmp;
    }

    Image image = LoadImage(image_path);     // Loaded in CPU memory (RAM)
    int maxDimen = image.height;
    if(image.width > image.height) maxDimen = image.width;
    *scale = ((float)image_size)/((float)maxDimen);
    if (texture == NULL)
        *texture = LoadTextureFromImage(image);          // Image converted to texture, GPU memory (VRAM)
    else{
        UnloadTexture(*texture);
        *texture = LoadTextureFromImage(image);
    }
    UnloadImage(image);  

    printf("load image path:%s\n", image_path);
}

bool isFileValid(char *path) {
    if(path == NULL) return false;
    int size = 0;
    while(path[size] != '\0') {
        size++;
    }
    int dotIndex = size -1;
    while(path[dotIndex] != '.'){
        dotIndex--;
    }
    char *png = "png";
    char *jpeg = "jpeg";
    char *jpg = "jpg";
    char *acceptableExtentions[3] = {png, jpeg, jpg};
    for(int i = dotIndex+1; i < size; i++) {
        bool match = false;
        for(int j = 0; j < 3; j++) {
            if(acceptableExtentions[j][i-dotIndex-1] == path[i]){
                match=true;
            }
        }
        if(match == false)
            return false;
    }
    return true;
}

int main(){
    
    // UnloadImage(img);
    const int screenWidth = 1280;
    const int screenHeight = 720;

    
    int padding = 80;
    int start_x = padding;
    int end_x = screenWidth - padding;
    int elementContainer = (end_x - start_x)/DEFAULT_K;
    int square_size = 80;
    int start_y = screenHeight - square_size - padding;


    int draw_y_start = padding + 20 + padding; 
    int draw_y_end = screenHeight - padding - 20 - 20 - square_size - padding;

    int draw_height = draw_y_end - draw_y_start;
    int image_size = draw_height;

    char *filePaths = 0; // We will register a maximum of filepaths

    InitWindow(screenWidth, screenHeight, "Dominant RgbColor");
    char *currentFilePath = (char *) RL_CALLOC(MAX_FILEPATH_SIZE, 1);;
    TextCopy(currentFilePath, DEFAULT_IMAGE_PATH);
    Color* dominantColors = (Color*)calloc(DEFAULT_K, DEFAULT_K);
    Texture2D texture;
    float textureScale = 1.0f;
    GetDominantColors(currentFilePath, image_size, &dominantColors, &texture, DEFAULT_K, &textureScale);


    if (texture.id == 0) {
        // Handle error
        // For example, print an error message and return.
        printf("Failed to load texture!\n");
        UnloadTexture(texture);  
        CloseWindow();
        return 0;
    }

    while (!WindowShouldClose())
    {
        // TODO: loading if k-mean clustering takes long time
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            if(droppedFiles.count > 1){
                // error
                printf("ERROR: Dropped to many files. Only one file drop is supported!\n");
                UnloadDroppedFiles(droppedFiles);
            } else if(!isFileValid(droppedFiles.paths[0])){
                // error
                printf("ERROR: Unsupported file! Only accepted extentions: jpeg/jpg, png \n");
                UnloadDroppedFiles(droppedFiles);
            } else{
                TextCopy(currentFilePath, droppedFiles.paths[0]);
                printf("dropped file:%s\n", currentFilePath);
                UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory
                GetDominantColors(currentFilePath, image_size, &dominantColors, &texture, DEFAULT_K, &textureScale);
            }
        }
        BeginDrawing();
            ClearBackground(WHITE);
            int exactWidth = texture.width * textureScale;
            Vector2 texturePos = {screenWidth/2 - exactWidth/2,
                draw_y_start};
            DrawTextureEx(
                texture,
                texturePos,
                0.0f,
                textureScale,
                WHITE
            );

            for(int i = 0; i < DEFAULT_K; i++){
                int cur_container_x = start_x + i*elementContainer;        
                int cur_x = cur_container_x + elementContainer/2 - square_size;
               

                DrawRectangle(cur_x, start_y, square_size, square_size, dominantColors[i]);
                DrawText(
                    TextFormat("(%d,%d,%d)", dominantColors[i].r, dominantColors[i].g,dominantColors[i].b),
                    cur_x+square_size/2, start_y+square_size+20,20, BLACK
                );
            }
            DrawText(
                TextFormat("(File: %s)", currentFilePath),
                 screenWidth/2 - padding, padding,20, BLACK
            );
        EndDrawing();
    }
    UnloadTexture(texture);  
    CloseWindow(); 
    return 0;
}