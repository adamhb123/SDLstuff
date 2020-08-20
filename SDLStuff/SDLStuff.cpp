#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <random>
#define PI 3.1415

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Color conversion 'borrowed' from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both        //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void(*mvf_non_loop_type)();

typedef void (*mvf_loop_type)();



typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    }
    else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if (in.r >= max)                           // > is bogus, just keeps compilor happy
        out.h = (in.g - in.b) / delta;        // between yellow & magenta
    else
        if (in.g >= max)
            out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
        else
            out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch (i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  End of borrowed code                                                                                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



struct Line {
    int xa, xb, ya, yb;
};

typedef std::vector<Line> Visual;


class MathematicalVisualizations {
private:
    int line_cursor = 0, visual_cursor = 0;
    
    void ResetCursors() { line_cursor = 0, visual_cursor = 0; }

 public:
     std::vector<Visual> visual_store;
     // Visual store size is only set when removing or adding a visual, as it is used to ensure we don't try looping through visual store when it is empty.
     int visual_store_size=-1;
     bool loop_color=true;
     int current_color[3] = { 0,255,255 }, background_color[3] = {0,0,0};
     SDL_Renderer* renderer;
     SDL_Window* window;
     float render_delay=0;
     int SCR_W=NULL, SCR_H=NULL, inc_factor = 2, scale = 1;
     MathematicalVisualizations(SDL_Window* _window, SDL_Renderer* _renderer) {
         renderer = _renderer;
         window = _window;
         SDL_GetWindowSize(window, &SCR_W, &SCR_H);
         SDL_SetRenderDrawColor(renderer, background_color[0], background_color[1], background_color[2],1);
     }
     // Begin of pattern methods
     void SpiralingLines(int degrees) {
         degrees += 2;
         Visual visual;
         int xa=0, xb=0, ya=0, yb=0;
         for (int i = 0; i < degrees; i++) {
             xb = (int)(round(cos(i*PI/180) * (scale)));
             yb = (int)(round(sin(i*PI/180) * (scale)));
             visual.push_back(Line{ xa + SCR_W / 2, xb + SCR_W / 2, ya + SCR_H / 2, yb + SCR_H / 2 });
             scale += inc_factor;
         }
         visual_store.push_back(visual);
         visual_store_size = visual_store.size();
     }
     // Not done am stupid
     void SpiralingLinesFillScreen(int rows, int columns, int degrees) {
         degrees += 2;
         int xa=0, xb=0, ya=0, yb=0;
         for (int y = 1; y <= columns; y++) {
             for (int x = 1; x <= rows; x++) {
                 Visual visual = {};
                 for (int i = 0; i < degrees; i++) {
                     xb = (int)(round(cos(i * PI / 180) * (scale)));
                     yb = (int)(round(sin(i * PI / 180) * (scale)));
                     visual.push_back(Line{ xa + x * (SCR_W / rows), xb, ya + y * (SCR_W / columns), yb });
                     scale += inc_factor;
                 }
                 visual_store.push_back(visual);
             }
         }
         visual_store_size = visual_store.size();
     }
     void RandomSpiralingLines(int degrees, int count) {
         std::random_device rd;
         std::mt19937 mt(rd());
         std::uniform_int_distribution<int> dist_x(0, SCR_W);
         std::uniform_int_distribution<int> dist_y(0, SCR_H);
         int center_x = dist_x(mt), center_y = dist_y(mt);
         for (int i = 0; i < count; i++) {



         }
         visual_store_size = visual_store.size();
     }
     void PingasCone(int starting_center_x, int starting_center_y, int rows) {
         int mouse_x, mouse_y;
         Visual visual;
         SDL_GetMouseState(&mouse_x, &mouse_y);
         int xa = starting_center_x, xb = 0, ya = starting_center_y, yb = 0;
         for (int i = 0; i < 362; i++) {
             Line curln = Line{xa + SCR_W / 2, xb + SCR_W / 2, ya + SCR_H / 2, yb + SCR_H / 2 };
             xb = (int)(round(cos(i * PI / 180) * (scale)));
             yb = (int)(round(sin(i * PI / 180) * (scale)));
             visual.push_back(curln);
         }
         visual_store.push_back(visual);
         visual_store_size = visual_store.size();
     }
     void EraseVisualByIndex(int index) {
         if (visual_store_size != 0) {
             visual_store.erase(visual_store.begin() + index);
             ResetCursors();
         }
         ResetCursors();
         Reset();
         visual_store_size = visual_store.size();
     }
     void PopBackVisual() {
         if (visual_store_size != 0) {
             visual_store.pop_back();
             ResetCursors();
             Reset();
         }
         visual_store_size = visual_store.size();
     }
     // End of pattern methods
     void ColorVariationStep() {
         hsv cur_col_conv = rgb2hsv(rgb{ (double)current_color[0]/255, (double)current_color[1]/255, (double)current_color[2]/255 });
         cur_col_conv.h += 1;
         rgb col_reconv = hsv2rgb(cur_col_conv);
         current_color[0] = floor(col_reconv.r * 255);
         current_color[1] = floor(col_reconv.g * 255);
         current_color[2] = floor(col_reconv.b * 255);

     }
     void RenderVisualizations() {
         SDL_Delay(render_delay);
         SDL_SetRenderDrawColor(renderer, current_color[0], current_color[1], current_color[2],1);
         if (visual_store_size != 0) {
             if (loop_color == true) ColorVariationStep();
             Line rendln = visual_store[visual_cursor][line_cursor];
             std::vector<std::vector<Line>>::size_type vsvect_sz = visual_store.size();
             SDL_RenderDrawLine(renderer, rendln.xa, rendln.ya, rendln.xb, rendln.yb);
             if (line_cursor == visual_store[visual_cursor].size() - 1) {
                 if (visual_cursor == vsvect_sz - 1) visual_cursor = 0;
                 else visual_cursor++;
                 line_cursor = 0;
             }
             else line_cursor++;
         }
         else {
             SDL_SetRenderDrawColor(renderer, background_color[0], background_color[1], background_color[2], 1);
             SDL_RenderClear(renderer);
         }
     }
     
     void Reset() {
         SDL_SetRenderDrawColor(renderer, background_color[0], background_color[1], background_color[2], 1);
         SDL_RenderClear(renderer);
         visual_store.empty();
     }
};

SDL_Texture* SafeLoadImg(SDL_Renderer* renderer,const char* file, bool &quit, SDL_Rect &out_rect, int pos_x = 0, int pos_y = 0) {
    SDL_Texture* image_as_texture = IMG_LoadTexture(renderer, file);
    int w, h;
    if (!image_as_texture) {
        std::cout << "Image loading error: " << IMG_GetError() << std::endl << "Resorting to fallback image..." << std::endl;
        image_as_texture = IMG_LoadTexture(renderer,"./resources/fallback.png");
        if (!image_as_texture) {
            std::cout << "Failed to load fallback image...total failure..." << std::endl;
            quit = true;
        }
    }

    if (SDL_QueryTexture(image_as_texture, NULL, NULL, &w, &h) != 0) 
        std::cout << "Rect retrieval error: " << IMG_GetError() << std::endl << "...total failure..." << std::endl;
    
    out_rect.x = pos_x;
    out_rect.y = pos_y;
    out_rect.w = w;
    out_rect.h = h;
    
    return image_as_texture;
}

struct TextRect {
    SDL_Texture* texture;
    SDL_Rect rect;
    bool display = true;
};

struct AnimatedTextRect {
    std::vector<SDL_Texture*> textures;
    SDL_Rect rect;
    int ms_per_frame;
    bool display = true;
};

template <class TextureMap>
void UnloadTextureFromMap(TextureMap texture_map, std::string texture_name) {
    SDL_DestroyTexture(texture_map[texture_name].texture);
    texture_map.erase(texture_name);
    std::cout << "Destroyed texture '" << texture_name << "' from a texture map." << std::endl;
}

void UnloadChunkFromMap(std::map<std::string, Mix_Chunk*> audio_map, std::string audio_name) {
    Mix_FreeChunk(audio_map[audio_name]);
    audio_map.erase(audio_name);
    std::cout << "Unloaded audio chunk '" << audio_name << "' from an audio map." << std::endl;
}


void SDLCleanup(SDL_Window* window, std::map<std::string, TextRect> plain_textures, std::map<std::string, Mix_Chunk*> sound_effects) {
    for (std::map<std::string, TextRect>::iterator it = plain_textures.begin(); it != plain_textures.end(); ++it) {
        UnloadTextureFromMap(plain_textures, it->first);
    }
    for (std::map<std::string, Mix_Chunk*>::iterator it = sound_effects.begin(); it != sound_effects.end(); ++it) {

    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    while (Mix_Init(0)) 
        Mix_Quit();
    
}


TextRect MakeTextRect(SDL_Renderer* renderer, const char* file, bool& quit) {
    SDL_Rect rect;
    TextRect text_rect;
    text_rect.texture = SafeLoadImg(renderer, file, quit, rect);
    text_rect.rect = rect;
    return text_rect;
}



int main(int argc, char *argb[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event sdl_event;
    std::string keyname;
    bool quit = false;
 


    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << SDL_GetError() << std::endl;
        quit = true;
    }

    window = SDL_CreateWindow("Bingotron", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
   

    renderer = SDL_CreateRenderer(window, -1, 0);

    if (window == NULL) {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        quit = true;
    }

    
    if (renderer == NULL) {
        std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
        quit = true;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        quit = true;
    }

    std::map<std::string, TextRect> current_plain_textures = {
        {"Bruh", MakeTextRect(renderer, "./resources/bruh.png", quit)}
    };

    std::map<std::string, AnimatedTextRect> current_animated_textures = {};

    std::map<std::string, Mix_Chunk*> current_sound_effects = {
        {"Tester", Mix_LoadWAV("./resources/mcostr.wav")}
    };
    //Mix_Chunk* testersound = Mix_LoadWAV("./resources/mcostr.wav");
    MathematicalVisualizations mathvis = MathematicalVisualizations(window,renderer);
    mathvis.scale = 1000000;
    mathvis.SpiralingLines(360);
    mathvis.PingasCone(100,100,2000);
    while (!quit) {
        
        mathvis.RenderVisualizations();

        //  Render all plain textures
        for (std::map<std::string, TextRect>::iterator it = current_plain_textures.begin(); it != current_plain_textures.end(); ++it) {
            if (it->second.display == true) {
                //SDL_RenderCopy(renderer, it->second.texture, NULL, &(it->second.rect));
            }
        }
        while (SDL_PollEvent(&sdl_event)) {

            if (sdl_event.type == SDL_QUIT) {
                std::cout << "Quitting..." << std::endl;
                quit = true;
            }
            else if (sdl_event.type == SDL_KEYDOWN) {
                std::cout << "inc: " << mathvis.inc_factor << std::endl;
                switch (sdl_event.key.keysym.sym) {
                case SDLK_RETURN:
                    mathvis.Reset();
                    break;
                case SDLK_HOME:
                    mathvis.render_delay += 1;
                    break;
                case SDLK_END:
                    if(mathvis.render_delay > 0) mathvis.render_delay -= 1;
                    break;

                case SDLK_KP_0:
                    mathvis.PopBackVisual();
                    std::cout << "Post-pop Visual count: " << mathvis.visual_store.size() << std::endl;
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDLCleanup(window, current_plain_textures, current_sound_effects);
    return 0;
}
