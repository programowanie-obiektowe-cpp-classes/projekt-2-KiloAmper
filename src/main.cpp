#include "PrintHello.hpp"
#include <SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL_mixer.h> 
#include <SDL_ttf.h>
#include <sstream>
// Je¿eli by³yby jakieœ problemy, to poni¿ej znajduje siê materia³
// z tym jak dzia³a gra, trzeba odtworzyæ z dŸwiêkiem.
// U¿ywane biblioteki: SDL,SDL_mixer,SDL_ttf
// https://youtu.be/HfC0YDwLN8k
// na nagraniu lekko przycina, ale to wina programu do nagrywania

// Sta³e definiuj¹ce wymiary okna
const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;

// Struktura opisuj¹ca obiekt w grze
struct Entity
{
    SDL_Texture* texture;
    SDL_Rect     rect;
    int          moveDirection; // 1 - w prawo, -1 - w lewo
};

struct Explosion
{
    SDL_Texture* texture;
    SDL_Rect     rect;
    int          frame; // Liczba klatek animacji
};


// Funkcja sprawdzaj¹ca kolizjê
bool checkCollision(const SDL_Rect& a, const SDL_Rect& b)
{
    return SDL_HasIntersection(&a, &b);
}

// Funkcja sprawdzaj¹ca kolizjê dwóch ³odzi
bool checkBoatCollision(const Entity& boat1, const Entity& boat2)
{
    return SDL_HasIntersection(&boat1.rect, &boat2.rect);
}

// Funkcja zmieniaj¹ca kierunek ruchu ³odzi
void changeBoatDirection(Entity& boat, SDL_Texture* boatRightTexture, SDL_Texture* boatLeftTexture)
{
    boat.moveDirection *= -1; // Zmiana kierunku (lewo <-> prawo)

    // Zmiana tekstury ³odzi w zale¿noœci od kierunku
    if (boat.moveDirection == 1)
    {
        boat.texture = boatRightTexture; // Ruch w prawo
    }
    else
    {
        boat.texture = boatLeftTexture; // Ruch w lewo
    }
}


// Funkcja sprawdzaj¹ca, czy nowa ³ódŸ zachodzi na istniej¹ce ³odzie
bool checkBoatOverlap(const Entity& newBoat, const std::vector< Entity >& boats)
{
    for (const auto& boat : boats)
    {
        if (checkBoatCollision(newBoat, boat))
        {
            return true; // Znaleziono nak³adaj¹ce siê ³odzie
        }
    }
    return false; // Brak nak³adaj¹cych siê ³odzi
}





int main(int argc, char* argv[])
{
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
        return -1;
    }

        // Inicjalizacja SDL_mixer
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0)
    {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

        // Inicjalizacja SDL_ttf
    if (TTF_Init() == -1)
    {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        std::cerr << "Failed to open audio: " << Mix_GetError() << "\n";
        Mix_Quit();
        SDL_Quit();
        return -1;
    }


    // Tworzenie okna i renderera
    SDL_Window* window = SDL_CreateWindow(
        "River Raid", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer)
    {
        std::cerr << "Failed to create window or renderer: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

// Za³aduj czcionkê
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << "\n";
        return -1;
    }


        // £adowanie dŸwiêków
    Mix_Chunk* shootSound     = Mix_LoadWAV("shoot.wav");     // DŸwiêk strza³u
    Mix_Chunk* explosionSound = Mix_LoadWAV("explosion.wav"); // DŸwiêk wybuchu
    Mix_Chunk* engineSound    = Mix_LoadWAV("engine.wav");// DŸwiêk silnika

       if (!shootSound || !explosionSound || !engineSound)
    {
        std::cerr << "Failed to load sound effects: " << Mix_GetError() << "\n";
        Mix_Quit();
        SDL_Quit();
        return -1;
    }


        // £adowanie muzyki
    Mix_Music* backgroundMusic = Mix_LoadMUS("wagner.wav"); //muzyka w tle
    if (!backgroundMusic)
    {
        std::cerr << "Failed to load background music: " << Mix_GetError() << "\n";
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

        // Rozpoczêcie odtwarzania muzyki w tle
    if (Mix_PlayMusic(backgroundMusic, -1) == -1) // -1 oznacza, ¿e muzyka bêdzie odtwarzana w pêtli
    {
        std::cerr << "Failed to play music: " << Mix_GetError() << "\n";
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }
   
   
    //ladowanie tla
    SDL_Surface* backgroundSurface = SDL_LoadBMP("sea_background.bmp");
    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    if (!backgroundTexture)
    {
        std::cerr << "Failed to create background texture: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

        // £adowanie grafiki wybuchu
    SDL_Surface* explosionSurface = SDL_LoadBMP("explosion.bmp");
    SDL_Texture* explosionTexture = SDL_CreateTextureFromSurface(renderer, explosionSurface);
    SDL_FreeSurface(explosionSurface);

    if (!explosionTexture)
    {
        std::cerr << "Failed to load explosion texture: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    // £adowanie grafik elementow BMP
    SDL_Surface* boatRightSurface  = SDL_LoadBMP("boat_right.bmp");
    SDL_Surface* boatLeftSurface   = SDL_LoadBMP("boat_left.bmp");
    SDL_Surface* planeSurface      = SDL_LoadBMP("plane.bmp");
    SDL_Surface* planeRightSurface = SDL_LoadBMP("plane_right.bmp");
    SDL_Surface* planeLeftSurface  = SDL_LoadBMP("plane_left.bmp");
    SDL_Surface* bulletSurface     = SDL_LoadBMP("bullet.bmp");


    //jakimœ cudem, nagle kod ponizej zaczal wywalac blad, mimo wczesniejszego dzialania.
    // Przestal dzialac w momencie dolaczenia dwoch grafik - boatRightSurface i boatLeftSurface
    // gdy byla jedna grafika i statki sie nie ruszaly to dzialalo
    //if (!planeSurface || !planeRightSurface || !planeLeftSurface || !bulletSurface || !boatRightSurface ||
    //    !boatLeftSurface);
    //{
    //    std::cerr << "Failed to load BMP:  " << SDL_GetError() << "\n";
    //    SDL_DestroyRenderer(renderer);
    //    SDL_DestroyWindow(window);
    //    SDL_Quit();
    //    return -1;
    //}

    // Tworzenie tekstur z grafik
    SDL_Texture* boatRightTexture  = SDL_CreateTextureFromSurface(renderer, boatRightSurface);
    SDL_Texture* boatLeftTexture   = SDL_CreateTextureFromSurface(renderer, boatLeftSurface);
    SDL_Texture* planeTexture      = SDL_CreateTextureFromSurface(renderer, planeSurface);
    SDL_Texture* planeRightTexture = SDL_CreateTextureFromSurface(renderer, planeRightSurface);
    SDL_Texture* planeLeftTexture  = SDL_CreateTextureFromSurface(renderer, planeLeftSurface);
    SDL_Texture* bulletTexture     = SDL_CreateTextureFromSurface(renderer, bulletSurface);

    // Zwalnianie powierzchni
    SDL_FreeSurface(boatRightSurface);
    SDL_FreeSurface(boatLeftSurface);
    SDL_FreeSurface(planeSurface);
    SDL_FreeSurface(planeRightSurface);
    SDL_FreeSurface(planeLeftSurface);
    SDL_FreeSurface(bulletSurface);

    if (!planeTexture || !planeRightTexture || !planeLeftTexture || !boatRightSurface || !boatLeftSurface ||
        !bulletTexture)
    {
        std::cerr << "Failed to create textures: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Inicjalizacja obiektów gry
    Entity                player = {planeTexture, {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 100, 50, 50}};
    std::vector< Entity > boats;
    std::vector< Entity > bullets;
    std::vector< Explosion > explosions; 

    // Zmienne gry
    bool      running = true;
    SDL_Event event;
    int       speed       = 5;  // Prêdkoœæ gracza
    int       boatSpeed   = 3;  // Prêdkoœæ ³odzi
    int       bulletSpeed = 10; // Prêdkoœæ pocisków
    int       score       = 0;  // Wynik gracza

    Uint32       lastShotTime = 0;   // Czas ostatniego strza³u
    const Uint32 shotCooldown = 200; // Odstêp miêdzy strza³ami w milisekundach

    SDL_Texture* currentTexture = planeTexture; // Obecna tekstura gracza

    srand(static_cast< unsigned >(time(0))); // Inicjalizacja generatora losowego
    const int explosionFrames = 10;
    // Pêtla gry
    while (running)
    {
        // Obs³uga zdarzeñ
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }


        // Przygotowanie tekstu
        std::string scoreText = "SCORE: "+ std::to_string(score);
        SDL_Color   textColor = {255, 255, 255, 255}; // Kolor tekstu: bia³y

// Tworzenie powierzchni tekstu
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (!scoreSurface)
        {
            std::cerr << "Failed to create text surface: " << TTF_GetError() << "\n";
            return -1;
        }

        // Tworzenie tekstury z powierzchni
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        if (!scoreTexture)
        {
            std::cerr << "Failed to create text texture: " << SDL_GetError() << "\n";
            SDL_FreeSurface(scoreSurface);
            return -1;
        }

        // Ustawienie pozycji tekstu (lewy górny róg)
        SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};


        // Sterowanie samolotem
        const Uint8* keystate          = SDL_GetKeyboardState(NULL);
        bool         movedHorizontally = false;

        if (keystate[SDL_SCANCODE_W] && player.rect.y > 0)
            player.rect.y -= speed;
        if (keystate[SDL_SCANCODE_S] && player.rect.y < SCREEN_HEIGHT - player.rect.h)
            player.rect.y += speed;

        if (keystate[SDL_SCANCODE_A] && player.rect.x > 0)
        {
            player.rect.x -= speed;
            currentTexture    = planeLeftTexture;
            movedHorizontally = true;
        }

        if (keystate[SDL_SCANCODE_D] && player.rect.x < SCREEN_WIDTH - player.rect.w)
        {
            player.rect.x += speed;
            currentTexture    = planeRightTexture;
            movedHorizontally = true;
        }

        if (!movedHorizontally)
        {
            currentTexture = planeTexture; // Powrót do domyœlnej tekstury
        }

        // Strzelanie z ograniczeniem czasu
        Uint32 currentTime = SDL_GetTicks(); // Aktualny czas w milisekundach
        if (keystate[SDL_SCANCODE_SPACE] && (currentTime - lastShotTime >= shotCooldown))
        {
            Mix_PlayChannel(-1, shootSound, 0); // Odtwarzanie strza³u na wolnym kanale
            bullets.push_back({bulletTexture, {player.rect.x + player.rect.w / 2 - 5, player.rect.y - 10, 10, 20}});
            lastShotTime = currentTime;
        }

        // Ruch pocisków
        for (auto& bullet : bullets)
        {
            bullet.rect.y -= bulletSpeed;
        }

        // Usuwanie pocisków poza ekranem
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(), [](const Entity& b) { return b.rect.y + b.rect.h < 0; }),
            bullets.end());

        // Dodawanie nowych ³odzi
        if (rand() % 100 < 2)
        { // Szansa na dodanie ³odzi w ka¿dej klatce
            //boats.push_back({boatTexture, {rand() % (SCREEN_WIDTH - 100), -100, 110, 25}, rand() % 2 == 0 ? 1 : -1});
            // Nowa ³ódŸ z losowym kierunkiem ruchu (1 to prawo, -1 to lewo)
            Entity newBoat;
            newBoat.rect          = {rand() % (SCREEN_WIDTH - 100), -100, 110, 25};
            newBoat.moveDirection = rand() % 2 == 0 ? 1 : -1;
            newBoat.texture       = (newBoat.moveDirection == 1) ? boatRightTexture : boatLeftTexture;
            // Sprawdzanie, czy ³ódŸ nie nak³ada siê na inne
            if (!checkBoatOverlap(newBoat, boats))
            {
                boats.push_back(newBoat); // Dodanie ³odzi do wektora, jeœli nie zachodzi na inne
            }
        }

// Poruszanie ³odzi w ró¿nych kierunkach
        for (auto& boat : boats)
        {
            boat.rect.y += boatSpeed;                      // Poruszanie w pionie (w dó³)
            boat.rect.x += boat.moveDirection * boatSpeed; // Poruszanie w poziomie (lewo/prawo)

            // Kolizja miêdzy ³odziami (zmiana kierunku po kolizji)
            for (auto& otherBoat : boats)
            {
                if (&boat != &otherBoat && checkCollision(boat.rect, otherBoat.rect))
                {
                    changeBoatDirection(boat, boatRightTexture, boatLeftTexture);
                    changeBoatDirection(otherBoat, boatRightTexture, boatLeftTexture); // Zmiana kierunku obu ³odzi
                }
            }

            // Sprawdzenie, czy ³ódŸ nie wysz³a poza ekran (w poziomie)
            if (boat.rect.x < 0 || boat.rect.x > SCREEN_WIDTH - boat.rect.w)
            {
                boat.moveDirection *= -1; // Zmiana kierunku na przeciwny
                boat.texture = (boat.moveDirection == 1) ? boatRightTexture : boatLeftTexture; // Zmiana tekstury
            }
        }

        // Usuwanie ³odzi poza ekranem
        boats.erase(
            std::remove_if(boats.begin(), boats.end(), [](const Entity& b) { return b.rect.y > SCREEN_HEIGHT; }),
            boats.end());

        // Kolizja: pocisk z ³odzi¹
        for (auto it = boats.begin(); it != boats.end();)
        {
            bool destroyed = false;
            for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();)
            {
                if (checkCollision(it->rect, bulletIt->rect))
                {
                    
                    Mix_PlayChannel(-1, explosionSound, 0); // Odtwarzanie wybuchu na wolnym kanale
                    explosions.push_back({explosionTexture, {it->rect.x, it->rect.y, 110, 40}, 0});
                    it        = boats.erase(it);
                    bulletIt  = bullets.erase(bulletIt);
                    destroyed = true;
                    score += 100; // Dodanie punktów
                    break;
                }
                else
                {
                    ++bulletIt;
                }
            }
            if (!destroyed)
                ++it;
        }

        // Kolizja: ³ódŸ z graczem
        for (const auto& boat : boats)
        {
            if (checkCollision(player.rect, boat.rect))
            {
                running = false; // Gra koñczy siê na kolizji
            }
        }

        // Aktualizacja wyniku i prêdkoœci
        score++;
        if (score % 500 == 0)
        { // Przyspieszenie gry co 500 punktów
            speed++;
            boatSpeed++;
        }


                // Animacja wybuchów
        for (auto& explosion : explosions)
        {
            explosion.frame++;
        }

        // Usuwanie wybuchów po animacji
        explosions.erase(std::remove_if(explosions.begin(),
                                        explosions.end(),
                                        [](const Explosion& e) { return e.frame >= explosionFrames; }),
                         explosions.end());

        // Renderowanie
        SDL_RenderClear(renderer);



        //Renderowanie tla
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        // Renderowanie gracza
        SDL_RenderCopy(renderer, currentTexture, NULL, &player.rect);

       // Renderowanie ³odzi
        for (const auto& boat : boats)
        {
            SDL_RenderCopy(renderer, boat.texture, NULL, &boat.rect);
        }

        // Renderowanie pocisków
        for (const auto& bullet : bullets)
        {
            SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
        }

                // Renderowanie wybuchów
        for (const auto& explosion : explosions)
        {
            SDL_RenderCopy(renderer, explosion.texture, NULL, &explosion.rect);
        }

        // Renderowanie tekstu
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    // Wyœwietlenie wyniku po zakoñczeniu gry
    std::cout << "Game Over! Your score: " << score << "\n";

    // Sprz¹tanie
    //SDL_FreeSurface(scoreSurface);
    //SDL_DestroyTexture(scoreTexture);
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(explosionSound);
    Mix_CloseAudio();
    TTF_CloseFont(font);
    SDL_DestroyTexture(explosionTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(planeTexture);
    SDL_DestroyTexture(planeRightTexture);
    SDL_DestroyTexture(planeLeftTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();

    return 0;
}
