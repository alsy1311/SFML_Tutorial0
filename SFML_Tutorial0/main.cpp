#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
#include <random>
#include <functional>

using namespace sf;

const int W = 1200;
const int H = 800;

float DEGTORAD = 0.017453f; 

float generator(float n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0, n);
    return dis(gen);
}

class Animation
{
public:

    float Frame, speed;

    Sprite sprite;

    std::vector<IntRect> frames;

    Animation() {

        Frame = 0;

        speed = 0;

    }

    Animation(Texture& t, int x, int y, int w, int h, int count, float Speed)
    {
        Frame = 0;
        speed = Speed;

        for (int i = 0; i < count; i++)
            frames.push_back(IntRect(x + i * w, y, w, h)); // вырезаем прямоугольники размером с картинку 

        sprite.setTexture(t);
        sprite.setOrigin(static_cast<float>(w / 2), static_cast<float>(h / 2));
        sprite.setTextureRect(frames[0]);
    }


    void update()
    {
        Frame += speed;
        int n = static_cast<int>(frames.size());
        if (Frame >= n) Frame -= n;
        if (n > 0) sprite.setTextureRect(frames[int(Frame)]);
    }

    bool isEnd()
    {
        return Frame + speed >= frames.size();
    }

};

class Entity
{
public:
    float x, y, dx, dy, R, angle;
    bool life;
    
    std::string name;
    Animation anim;

    Entity()
    {
        x = 0;
        y = 0;
        dx = 0;
        dy = 0;
        R = 0;
        angle = 0;
        life = 1;
    }

    void settings(Animation& a, int X, int Y, float Angle = 0, int radius = 1)
    {
        anim = a;
        x = static_cast<float>(X); y = static_cast<float>(Y);
        angle = Angle;
        R = static_cast<float>(radius);
    }

    virtual void update() {};

    void draw(RenderWindow& app)
    {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);

        CircleShape circle(R);
        circle.setFillColor(Color(255, 0, 0, 170));
        circle.setPosition(x, y);
        circle.setOrigin(R, R);
        //app.draw(circle);
    }

    virtual ~Entity() {};
};


class asteroid : public Entity
{
public:
    asteroid()
    {
        dx = generator(8.f) - 4;
        dy = generator(8.f) - 4;
        name = "asteroid";
    }

    void update()
    {
        x += dx;
        y += dy;

        if (x > W) x = 0;  if (x < 0) x = W;
        if (y > H) y = 0;  if (y < 0) y = H;
    }

};


class bullet : public Entity
{
public:
    bullet()
    {
        name = "bullet";
    }

    void  update()
    {
        dx = cos(angle * DEGTORAD) * 6;
        dy = sin(angle * DEGTORAD) * 6;
        // angle+=rand()%7-3;  /*try this*/
        x += dx;
        y += dy;

        if (x > W || x<0 || y>H || y < 0) life = 0;
    }

};


class player : public Entity
{
public:
    int countLifes;
    bool thrust;
    int counter;

    player()
    {
        counter = 0;
        countLifes = 3;
        name = "player";
    }

    void update()
    {
        if (thrust)
        {
            dx += cos(angle * DEGTORAD) * 0.2f;
            dy += sin(angle * DEGTORAD) * 0.2f;
        }
        else
        {
            dx *= 0.99f;
            dy *= 0.99f;
        }

        int maxSpeed = 15;
        float speed = sqrt(dx * dx + dy * dy);
        if (speed > maxSpeed)
        {
            dx *= maxSpeed / speed;
            dy *= maxSpeed / speed;
        }

        x += dx;
        y += dy;

        if (x > W) x = 0; if (x < 0) x = W;
        if (y > H) y = 0; if (y < 0) y = H;
    }

};


bool isCollide(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    return (b->x - a->x) * (b->x - a->x) +
        (b->y - a->y) * (b->y - a->y) <
        (a->R + b->R) * (a->R + b->R);
}


int main()
{


    RenderWindow app(VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    Texture t1, t2, t3, t4, t5, t6, t7;
    t1.loadFromFile("resources/spaceship.png");
    t2.loadFromFile("resources/background.jpg");
    t3.loadFromFile("resources/explosions/type_C.png");
    t4.loadFromFile("resources/rock.png");
    t5.loadFromFile("resources/fire_blue.png");
    t6.loadFromFile("resources/rock_small.png");
    t7.loadFromFile("resources/explosions/type_B.png");

    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);

    //Animation(Texture& t, int x, int y, int w, int h, int count, float Speed)
    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5f);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2f);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2f);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8f);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0.f);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0.f);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5f);


    std::list<std::shared_ptr<Entity>> entities;

    for (int i = 0; i < 15; i++)
    {
        auto a = std::make_shared<asteroid>();
        a -> settings(sRock, static_cast<int>(generator(W)), static_cast<int>(generator(H)), generator(360.f), 25);
        entities.push_back(a);
    }

    auto p = std::make_shared<player>();
    p -> settings(sPlayer, 200, 200, 0, 20);
    entities.push_back(p);

    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space)
                {
                    auto b = std::make_shared<bullet>();
                    b->settings(sBullet, static_cast<int>(p->x), static_cast<int>(p->y), p->angle, 10);
                    entities.push_back(b);
                }
        }

        if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += 3;
        if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle -= 3;
        if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust = true;
        else p->thrust = false;


        for (auto a : entities)
            for (auto b : entities)
            {
                if (a->name == "asteroid" && b->name == "bullet")
                    if (isCollide(a, b))
                    {
                        p->counter += 1;
                        a -> life = false;
                        b -> life = false;
                   
                        auto e = std::make_shared<Entity>();
                        e->settings(sExplosion, static_cast<int>(a->x), static_cast<int>(a->y));
                        e->name = "explosion";
                        entities.push_back(e);

                        for (int i = 0; i < 2; i++)
                        {
                            if (a->R == 15) continue;
                            auto e = std::make_shared<asteroid>();
                            e->settings(sRock_small, static_cast<int>(a->x), static_cast<int>(a->y), generator(360.f), 15);
                            entities.push_back(e);
                        }

                    }

                if (a->name == "player" && b->name == "asteroid")
                    if (isCollide(a, b))
                    {
                        p->counter += 1;
                     
                        b->life = false;
                        auto e = std::make_shared<Entity>();
                        e->settings(sExplosion_ship, static_cast<int>(a->x), static_cast<int>(a->y));
                        e->name = "explosion";
                        entities.push_back(e);
                        p->countLifes += -1;
                        if (p -> countLifes == 0) {
                            p->life = false;
                            app.close();
                        }
                        else {
                            p->settings(sPlayer, W / 2, H / 2, 0, 20);
                            p->dx = 0; p->dy = 0;
                        }
                    }
            }


        if (p->thrust)  p->anim = sPlayer_go;
        else   p->anim = sPlayer;


        for (auto e : entities)
            if (e->name == "explosion")
                if (e->anim.isEnd()) { e->life = 0; }

        if (rand() % 150 == 0)
        {
            auto a = std::make_shared<asteroid>();
            a->settings(sRock, 0, static_cast<int>(generator(H)), (generator(360.f)), 25);
            entities.push_back(a);
        }

        for (auto i = entities.begin(); i != entities.end();)
        {
            auto e = *i;

            e->update();
            e->anim.update();

            if (e->life == false) { i = entities.erase(i); }
            else i++;
        }

        Font font;
        font.loadFromFile("17857.ttf");
        Text text("Lifes: " + std::to_string(p->countLifes) + "\n" + "Score: " + std::to_string(p->counter), font);
        text.setCharacterSize(20);
        text.setStyle(sf::Text::Bold);
        
        text.setPosition(100, 750);
        // Рисуем это
       

        //////draw//////
        app.draw(background);
        app.draw(text);
        for (auto i : entities) i->draw(app);
        
        app.display();
    }

    return 0;
}
