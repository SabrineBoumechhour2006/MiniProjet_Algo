// main.cpp — Hanoi GUI (menu, history, pastel disks, capsule buttons, circle +/-)
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace std;

struct MenuResult { int diskCount; bool recursive; };
struct HistoryEntry { int n; bool recursive; double timeFinal; };

static vector<pair<char,char>> moves;
static vector<HistoryEntry> history;

// Helper: format double
string formatDouble(double v, int prec=6){
    ostringstream ss; ss<<fixed<<setprecision(prec)<<v; return ss.str();
}

// Helper: check contains for rectangle shapes
bool containsRect(const sf::RectangleShape &r, sf::Vector2i m){
    return r.getGlobalBounds().contains((float)m.x,(float)m.y);
}
// Helper: check contains for circle shapes
bool containsCircle(const sf::CircleShape &c, sf::Vector2i m){
    return c.getGlobalBounds().contains((float)m.x,(float)m.y);
}

// Draw capsule (rectangle + left/right circles)
void drawCapsule(sf::RenderTarget &target, const sf::RectangleShape &rect, const sf::Color &fill){
    // body rect
    sf::RectangleShape body = rect;
    body.setFillColor(fill);
    target.draw(body);

    // left & right circles
    float h = rect.getSize().y;
    float r = h/2.f;
    sf::CircleShape left(r), right(r);
    left.setFillColor(fill); right.setFillColor(fill);
    left.setOrigin(r,r); right.setOrigin(r,r);
    left.setPosition(rect.getPosition().x, rect.getPosition().y + r);
    right.setPosition(rect.getPosition().x + rect.getSize().x, rect.getPosition().y + r);
    target.draw(left); target.draw(right);
}

// Draw capsule with centered text
void drawCapsuleText(sf::RenderTarget &target, const sf::RectangleShape &rect, const sf::Color &fill,
                     const sf::Font &font, const string &text, unsigned int size, const sf::Color &textColor = sf::Color::White){
    drawCapsule(target, rect, fill);
    sf::Text t(text, font, size);
    t.setFillColor(textColor);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    t.setPosition(rect.getPosition() + rect.getSize()*0.5f);
    target.draw(t);
}

// Hanoi recursive moves generator
void hanoiRec(int n, char A, char C, char B){
    if(n != 0){
        hanoiRec(n-1, A, B, C);
        moves.push_back({A, C}); // Move(A, C)
        hanoiRec(n-1, B, C, A);
    }
}
// Hanoi iterative (simulation) moves generator
void hanoiIter(int n, char A, char C, char B){
    struct Peg { vector<int> s; };
    Peg pegA, pegB, pegC;
    for(int i=n;i>=1;--i) pegA.s.push_back(i);

    auto mv = [&](Peg& P1, Peg& P2, char c1, char c2){
        if(P1.s.empty()){
            moves.push_back({c2,c1});
            P1.s.push_back(P2.s.back()); P2.s.pop_back();
        } else if(P2.s.empty()){
            moves.push_back({c1,c2});
            P2.s.push_back(P1.s.back()); P1.s.pop_back();
        } else if(P1.s.back() < P2.s.back()){
            moves.push_back({c1,c2});
            P2.s.push_back(P1.s.back()); P1.s.pop_back();
        } else {
            moves.push_back({c2,c1});
            P1.s.push_back(P2.s.back()); P2.s.pop_back();
        }
    };

    char p1 = A, p2 = B, p3 = C;
    if(n % 2 == 0) swap(p2,p3);

    long long total = (1LL<<n) - 1;
    for(long long i=1;i<=total;++i){
        if(i%3==1) mv(pegA, pegC, p1, p3);
        else if(i%3==2) mv(pegA, pegB, p1, p2);
        else mv(pegB, pegC, p2, p3);
    }
}

void placeDisk(sf::RectangleShape &d, char peg, int heightIndex){
    float x = (peg=='A'?200.f:(peg=='B'?450.f:700.f));
    float w = d.getSize().x;
    d.setPosition(x - w/2.f + 5.f, 450.f - heightIndex*22.f);
}

/* ---------------- History Window ---------------- */
void runHistoryWindow(const sf::Font &font){
    const int W = 750, H = 550;
    sf::RenderWindow w(sf::VideoMode(W,H), "History");
    w.setFramerateLimit(60);

    sf::Text title("Execution History", font, 26);
    title.setFillColor(sf::Color(30,30,40));
    title.setPosition(20,10);

    float scroll=0.f;
    const float cardH = 78.f;
    const float spacing = 14.f;

    while(w.isOpen()){
        sf::Event ev;
        while(w.pollEvent(ev)){
            if(ev.type == sf::Event::Closed){ w.close(); return; }
            if(ev.type == sf::Event::MouseWheelScrolled){
                scroll -= ev.mouseWheelScroll.delta * 30.f;
                if(scroll < 0) scroll = 0;
            }
        }

        w.clear(sf::Color(245,245,250));

        w.draw(title);

        float y = 60.f - scroll;
        for(size_t i=0;i<history.size();++i){
            const HistoryEntry &he = history[i];
            sf::RectangleShape card(sf::Vector2f(W-80.f, cardH));
            card.setPosition(40.f, y);
            card.setFillColor(sf::Color::White);
            card.setOutlineThickness(1.5f);
            card.setOutlineColor(sf::Color(210,210,210));

            sf::Text line1("n = " + to_string(he.n) + "   Method: " + (he.recursive ? "Recursive" : "Iterative"),
                           font, 18);
            line1.setFillColor(sf::Color(20,20,30));
            line1.setPosition(60.f, y + 10.f);

            sf::Text line2("Time: " + formatDouble(he.timeFinal,6) + " s", font, 16);
            line2.setFillColor(sf::Color(90,90,110));
            line2.setPosition(60.f, y + 38.f);

            // small color badge
            sf::RectangleShape badge(sf::Vector2f(12,12));
            badge.setPosition(40.f + card.getSize().x - 40.f, y + 12.f);
            badge.setFillColor(he.recursive ? sf::Color(190,150,240) : sf::Color(180,180,180));

            w.draw(card);
            w.draw(line1);
            w.draw(line2);
            w.draw(badge);

            y += cardH + spacing;
        }

        sf::Text foot("Use mouse wheel to scroll", font, 14);
        foot.setFillColor(sf::Color(140,140,140));
        foot.setPosition(14, H-28);
        w.draw(foot);

        w.display();
    }
}

/* ---------------- Menu ---------------- */
MenuResult runMenu(){
    const int W = 900, H = 600;
    sf::RenderWindow win(sf::VideoMode(W,H), "Hanoi - Menu");
    win.setFramerateLimit(60);

    sf::Font font;
    if(!font.loadFromFile("Roboto-Regular.ttf")){
        // if missing, return defaults (no console prints)
        return {3,true};
    }

    int diskCount = 3;
    bool recursive = true;

    // texts
    sf::Text title("Tower of Hanoi", font, 44);
    title.setFillColor(sf::Color(30,30,40));
    title.setPosition(260, 30);

    sf::Text diskText("Number of disks: 3", font, 26);
    diskText.setFillColor(sf::Color(40,40,40));
    diskText.setPosition(300, 140);

    // plus/minus - use circles violet pastel (option 3)
    sf::CircleShape minusCircle(30.f);
    minusCircle.setPosition(260,220);
    minusCircle.setFillColor(sf::Color(210,180,240)); // violet pastel

    sf::Text minusT("-", font, 38);
    minusT.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = minusT.getLocalBounds();
        minusT.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        minusT.setPosition(minusCircle.getPosition() + sf::Vector2f(minusCircle.getRadius(), minusCircle.getRadius()-4));
    }

    sf::CircleShape plusCircle(30.f);
    plusCircle.setPosition(580,220);
    plusCircle.setFillColor(sf::Color(210,180,240)); // violet pastel

    sf::Text plusT("+", font, 38);
    plusT.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = plusT.getLocalBounds();
        plusT.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        plusT.setPosition(plusCircle.getPosition() + sf::Vector2f(plusCircle.getRadius(), plusCircle.getRadius()-4));
    }

    // method selection capsules (rounded) — recursive selected violet medium (option 2)
    sf::RectangleShape recChoice(sf::Vector2f(240,72)); recChoice.setPosition(140,330);
    sf::RectangleShape iterChoice(sf::RectangleShape(sf::Vector2f(240,72))); iterChoice.setPosition(520,330);

    sf::Text recTxt("Recursive", font, 28);
    recTxt.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = recTxt.getLocalBounds();
        recTxt.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        recTxt.setPosition(recChoice.getPosition() + recChoice.getSize()*0.5f);
    }

    sf::Text iterTxt("Iterative", font, 28);
    iterTxt.setFillColor(sf::Color::White);
    {
        sf::FloatRect b = iterTxt.getLocalBounds();
        iterTxt.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        iterTxt.setPosition(iterChoice.getPosition() + iterChoice.getSize()*0.5f);
    }

    // START and HISTORY (oval capsules)
    sf::RectangleShape startRect(sf::Vector2f(220,70));
    startRect.setPosition(160, 460);
    sf::RectangleShape historyRect(sf::RectangleShape(sf::Vector2f(220,70)));
    historyRect.setPosition(520, 460);

    // Colors per your choices
    sf::Color startColor(255, 200, 220);      // rose clair
    sf::Color historyColor(200, 235, 255);    // bleu ciel clair
    sf::Color backColor(60, 140, 255);        // bleu vif
    sf::Color recSelColor(180, 140, 220);     // violet moyen
    sf::Color iterUnselColor(170, 170, 170);  // grey for unselected

    while (win.isOpen()){
        sf::Event ev;
        while (win.pollEvent(ev)){
            if (ev.type == sf::Event::Closed){ win.close(); exit(0); }
            if (ev.type == sf::Event::MouseButtonPressed){
                sf::Vector2i m = sf::Mouse::getPosition(win);

                // minus circle
                if (containsCircle(minusCircle, m) && diskCount > 1){
                    diskCount--;
                    diskText.setString("Number of disks: " + to_string(diskCount));
                }
                // plus circle, limit to 8
                if (containsCircle(plusCircle, m) && diskCount < 8){
                    diskCount++;
                    diskText.setString("Number of disks: " + to_string(diskCount));
                }

                // method selecting by clicking the capsule bounding rect
                if (recChoice.getGlobalBounds().contains((float)m.x,(float)m.y)) recursive = true;
                if (iterChoice.getGlobalBounds().contains((float)m.x,(float)m.y)) recursive = false;

                // Start
                if (startRect.getGlobalBounds().contains((float)m.x,(float)m.y)){
                    win.close();
                    return {diskCount, recursive};
                }
                // History
                if (historyRect.getGlobalBounds().contains((float)m.x,(float)m.y)){
                    runHistoryWindow(font);
                }
            }
        }

        // set capsule fills
        recChoice.setFillColor(recursive ? recSelColor : iterUnselColor);
        iterChoice.setFillColor(!recursive ? recSelColor : iterUnselColor);

        // draw UI
        win.clear(sf::Color(245,245,250));

        win.draw(title);
        win.draw(diskText);

        // plus/minus circles and their text
        win.draw(minusCircle);
        win.draw(minusT);
        win.draw(plusCircle);
        win.draw(plusT);

        // draw capsules
        drawCapsule(win, recChoice, recChoice.getFillColor());
        drawCapsule(win, iterChoice, iterChoice.getFillColor());
        // centered texts are pre-positioned
        win.draw(recTxt);
        win.draw(iterTxt);

        // draw start/history capsules with text
        drawCapsuleText(win, startRect, startColor, font, "START", 24);
        drawCapsuleText(win, historyRect, historyColor, font, "HISTORY", 20);

        // small hint
        sf::Text hint("Max 8 disks for graphic animation", font, 14);
        hint.setFillColor(sf::Color(120,120,120));
        hint.setPosition(300, 200);
        win.draw(hint);

        win.display();
    }

    return {3, true};
}

/* ---------------- Animation ---------------- */
void runAnimation(int n, bool recursiveMethod, const sf::Font &font){
    sf::RenderWindow win(sf::VideoMode(900,600), "Hanoi Animation");
    win.setFramerateLimit(60);

    sf::Text timerText("", font, 22);
    timerText.setFillColor(sf::Color(20,20,30));
    timerText.setPosition(10, 10);

    // BACK button (blue vif)
    sf::RectangleShape backRect(sf::Vector2f(220,56));
    backRect.setPosition(640, 18);
    backRect.setFillColor(sf::Color(60,140,255));

    // rods
    sf::RectangleShape rod(sf::Vector2f(10,250)); rod.setFillColor(sf::Color(90,60,20));
    sf::RectangleShape Arod=rod, Brod=rod, Crod=rod;
    Arod.setPosition(200,200); Brod.setPosition(450,200); Crod.setPosition(700,200);

    // create disks with pastel palette (soft)
    vector<sf::RectangleShape> disks;
    vector<int> pegA, pegB, pegC ;
    
    for (int i = 0; i < n; i++) {
    float w = 160 - i * 20;
    sf::RectangleShape d(sf::Vector2f(w, 20));

    // Palette style "aurore"
    sf::Uint8 r = 255 - i * 15;  // du rose/orangé lumineux
    sf::Uint8 g = 180 + i * 10;  // léger vert/orangé
    sf::Uint8 b = 200 - i * 20;  // bleu/violet clair
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    d.setFillColor(sf::Color(r, g, b));

    d.setPosition(200 - w / 2 + 5, 450 - i * 22);
    disks.push_back(d);
    pegA.push_back(i);
}
    moves.clear();
    if (recursiveMethod) hanoiRec(n,'A','C','B'); else hanoiIter(n,'A','C','B');

    int step = 0;
    bool finished = false;
    bool saved = false;
    const float speed = 0.35f; // animation delay per move

    sf::Clock timerClock;
    sf::Clock moveClock;

    while (win.isOpen()){
        sf::Event ev;
        while (win.pollEvent(ev)){
            if (ev.type == sf::Event::Closed){ win.close(); exit(0); }
        }

        // update timer only while running
        if (!finished){
            double t = timerClock.getElapsedTime().asSeconds();
            timerText.setString("Time: " + formatDouble(t,6) + " s");
        }

        // Back button: if clicked before finishing, save current approximate time
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i mp = sf::Mouse::getPosition(win);
            if (backRect.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                if (!finished && !saved){
                    double t = timerClock.getElapsedTime().asSeconds();
                    history.push_back({n, recursiveMethod, t});
                    saved = true;
                }
                win.close();
                return;
            }
        }

        // play moves at interval
        if (!finished && step < (int)moves.size()){
            if (moveClock.getElapsedTime().asSeconds() > speed){
                char f = moves[step].first;
                char t = moves[step].second;
                vector<int>* src = (f=='A'?&pegA:(f=='B'?&pegB:&pegC));
                vector<int>* dst = (t=='A'?&pegA:(t=='B'?&pegB:&pegC));
                if (!src->empty()){
                    int id = src->back(); src->pop_back();
                    placeDisk(disks[id], t, dst->size());
                    dst->push_back(id);
                }
                ++step;
                moveClock.restart();
            }
        } else if (!finished){
            // finished now exactly when all moves applied
            finished = true;
            double finalT = timerClock.getElapsedTime().asSeconds();
            timerText.setString("Final: " + formatDouble(finalT,6) + " s");
            if (!saved){
                history.push_back({n, recursiveMethod, finalT});
                saved = true;
            }
        }

        // render
        win.clear(sf::Color::White);
        win.draw(Arod); win.draw(Brod); win.draw(Crod);
        for (auto &d : disks) win.draw(d);

        // timer + back (capsule with text)
        win.draw(timerText);
        drawCapsuleText(win, backRect, backRect.getFillColor(), font, "BACK TO MENU", 18);

        win.display();
    }
}

/* ---------------- main ---------------- */
int main(){
    sf::Font font;
    if(!font.loadFromFile("Roboto-Regular.ttf")){
        // cannot render GUI without font; exit quietly
        return 1;
    }

    while (true){
        MenuResult R = runMenu();
        // always graphical; n limited to 8 in menu
        runAnimation(R.diskCount, R.recursive, font);
    }

    return 0;
}
