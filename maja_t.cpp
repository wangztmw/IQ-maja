#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <map>
#include <set>
using namespace std;

// 麻将牌类型
enum class Suit { CHARACTER, BAMBOO, DOT, WIND, DRAGON };
const vector<string> SUIT_NAMES = {"万", "条", "筒", "风", "箭"};
const vector<string> WIND_NAMES = {"东", "南", "西", "北"};
const vector<string> DRAGON_NAMES = {"中", "发", "白"};

// 麻将牌类
class MahjongTile {
public:
    Suit suit;
    int rank; // 对于普通牌是1-9，风牌(0:东,1:南,2:西,3:北)，箭牌(0:中,1:发,2:白)
    
    MahjongTile(Suit s, int r) : suit(s), rank(r) {}
    
    string toString() const {
        if (suit == Suit::WIND) {
            return WIND_NAMES[rank] + "风";
        } else if (suit == Suit::DRAGON) {
            return DRAGON_NAMES[rank] + "箭";
        }
        return to_string(rank + 1) + SUIT_NAMES[static_cast<int>(suit)];
    }
    
    bool operator==(const MahjongTile& other) const {
        return suit == other.suit && rank == other.rank;
    }
    
    bool operator<(const MahjongTile& other) const {
        if (suit != other.suit) return static_cast<int>(suit) < static_cast<int>(other.suit);
        return rank < other.rank;
    }
};

// 玩家类
class Player {
public:
    int id;
    vector<MahjongTile> hand;
    vector<MahjongTile> discarded;
    
    Player(int id) : id(id) {}
    
    void drawTile(MahjongTile tile) {
        hand.push_back(tile);
        sort(hand.begin(), hand.end());
    }
    
    MahjongTile discardTile(size_t index) {
        MahjongTile discardedTile = hand[index];
        hand.erase(hand.begin() + index);
        discarded.push_back(discardedTile);
        return discardedTile;
    }
    
    void displayHand() const {
        cout << "玩家" << id << "的手牌: ";
        for (const auto& tile : hand) {
            cout << tile.toString() << " ";
        }
        cout << endl;
    }
};

// 麻将游戏类
class MahjongGame {
private:
    vector<MahjongTile> wall;
    vector<Player> players;
    int currentPlayer;
    MahjongTile lastDrawnTile = {Suit::CHARACTER, -1}; // 无效牌
    
    // 初始化麻将牌
    void initializeTiles() {
        // 添加万、条、筒 (每种4张)
        for (int s = 0; s < 3; s++) {
            for (int r = 0; r < 9; r++) {
                for (int i = 0; i < 4; i++) {
                    wall.push_back(MahjongTile(static_cast<Suit>(s), r));
                }
            }
        }
        
        // 添加风牌 (每种4张)
        for (int r = 0; r < 4; r++) {
            for (int i = 0; i < 4; i++) {
                wall.push_back(MahjongTile(Suit::WIND, r));
            }
        }
        
        // 添加箭牌 (每种4张)
        for (int r = 0; r < 3; r++) {
            for (int i = 0; i < 4; i++) {
                wall.push_back(MahjongTile(Suit::DRAGON, r));
            }
        }
    }
    
    // 洗牌
    void shuffleTiles() {
        static mt19937 rng(random_device{}());
        shuffle(wall.begin(), wall.end(), rng);
    }
    
    // 发牌
    void dealTiles() {
        // 每人13张牌，共4轮
        for (int i = 0; i < 13; i++) {
            for (int p = 0; p < 4; p++) {
                players[p].drawTile(wall.back());
                wall.pop_back();
            }
        }
    }
    
public:
    MahjongGame() : currentPlayer(0) {
        for (int i = 0; i < 4; i++) {
            players.emplace_back(i + 1);
        }
        initializeTiles();
        shuffleTiles();
        dealTiles();
    }
    
    // 显示当前状态
    void displayGameState() const {
        cout << "\n当前墙牌数量: " << wall.size() << endl;
        players[currentPlayer].displayHand();
        
        cout << "弃牌堆: ";
        for (const auto& tile : players[currentPlayer].discarded) {
            cout << tile.toString() << " ";
        }
        cout << endl;
    }
    
    // 当前玩家摸牌
    void drawTile() {
        if (!wall.empty()) {
            lastDrawnTile = wall.back();
            players[currentPlayer].drawTile(lastDrawnTile);
            wall.pop_back();
            cout << "玩家" << players[currentPlayer].id << "摸到: " 
                 << lastDrawnTile.toString() << endl;
        }
    }
    
    // 当前玩家打牌
    void discardTile(int index) {
        MahjongTile discarded = players[currentPlayer].discardTile(index);
        cout << "玩家" << players[currentPlayer].id << "打出: " 
             << discarded.toString() << endl;
        
        // 检查是否胡牌
        if (canWin(players[currentPlayer].hand)) {
            cout << "\n玩家" << players[currentPlayer].id << "胡牌了！游戏结束！" << endl;
            exit(0);
        }
        
        // 轮到下一位玩家
        currentPlayer = (currentPlayer + 1) % 4;
    }
    
    // 检查是否胡牌 (简化的胡牌规则)
    bool canWin(const vector<MahjongTile>& hand) {
        // 手牌必须为14张 (13张 + 刚摸到的1张)
        if (hand.size() != 14) return false;
        
        // 尝试找将牌 (一对相同的牌)
        for (size_t i = 0; i < hand.size() - 1; i++) {
            if (hand[i] == hand[i+1]) {
                vector<MahjongTile> remaining;
                for (size_t j = 0; j < hand.size(); j++) {
                    if (j != i && j != i+1) {
                        remaining.push_back(hand[j]);
                    }
                }
                
                // 检查剩下的牌是否能组成顺子或刻子
                if (canFormMeldSets(remaining)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // 检查是否能组成顺子或刻子
    bool canFormMeldSets(vector<MahjongTile> tiles) {
        if (tiles.empty()) return true;
        sort(tiles.begin(), tiles.end());
        
        // 尝试找刻子 (三个相同的牌)
        if (tiles.size() >= 3) {
            if (tiles[0] == tiles[1] && tiles[1] == tiles[2]) {
                vector<MahjongTile> newTiles(tiles.begin() + 3, tiles.end());
                if (canFormMeldSets(newTiles)) {
                    return true;
                }
            }
        }
        
        // 尝试找顺子 (只适用于万、条、筒)
        if (tiles[0].suit != Suit::WIND && tiles[0].suit != Suit::DRAGON) {
            // 查找连续牌
            MahjongTile next1(tiles[0].suit, tiles[0].rank + 1);
            MahjongTile next2(tiles[0].suit, tiles[0].rank + 2);
            
            auto it1 = find(tiles.begin(), tiles.end(), next1);
            auto it2 = find(tiles.begin(), tiles.end(), next2);
            
            if (it1 != tiles.end() && it2 != tiles.end()) {
                vector<MahjongTile> newTiles;
                bool removedFirst = false;
                for (const auto& tile : tiles) {
                    if (!removedFirst && tile == tiles[0]) {
                        removedFirst = true;
                        continue;
                    }
                    if (tile == next1) {
                        next1 = {Suit::CHARACTER, -1}; // 标记已移除
                        continue;
                    }
                    if (tile == next2) {
                        next2 = {Suit::CHARACTER, -1}; // 标记已移除
                        continue;
                    }
                    newTiles.push_back(tile);
                }
                if (canFormMeldSets(newTiles)) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // 游戏主循环
    void play() {
        cout << "麻将游戏开始！" << endl;
        
        while (!wall.empty()) {
            cout << "\n--- 玩家" << players[currentPlayer].id << "的回合 ---" << endl;
            drawTile();
            displayGameState();
            
            // 简单AI: 打出最后摸到的牌
            if (!players[currentPlayer].hand.empty()) {
                // 查找刚摸到的牌的位置
                auto it = find(players[currentPlayer].hand.begin(), 
                              players[currentPlayer].hand.end(), lastDrawnTile);
                int index = distance(players[currentPlayer].hand.begin(), it);
                discardTile(index);
            }
        }
        
        cout << "墙牌已摸完，流局！" << endl;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    MahjongGame game;
    game.play();
    return 0;
}
