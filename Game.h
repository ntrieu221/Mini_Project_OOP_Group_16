#ifndef GAME_H
#define GAME_H
#include <bits/stdc++.h>

using namespace std;

//Trạng thái của quân cờ, dùng để đánh dấu các trạng thái của quân cờ
enum State { IN_BASE, ON_BOARD, FINISHED }; 

//Thông tin các nước đi
struct Move {
    int horseIndex; //Chỉ số của các quân cờ (0-3) 
    int targetPos; //Vị trí mà quân cờ sẽ đến
    bool is_raquan; //Ra quân
    bool is_kill; //Đá
    bool is_finish; //Về đích
    bool is_safe; //Kiểm tra xem quân cờ có đang an toàn không
    int khoangcach_toidich; //Quãng đường còn lại để về đích
};

class Horse {
public: 
    int position; //Vị trí hiện tại của quân cờ (0-47)
    // -1 nếu ở trong chuồng hoặc đã về đích
    State state; //Trạng thái của quân cờ
    int soBuocdi; // Tổng số bước đã đi tính từ lúc ra quân
    Horse(); //Constructor
};

//Class đại diện cho Người chơi
class Player {
public: 
    int id; //ID của người chơi (0-3)
    int type;
    //Tượng trưng cho 4 lối chơi:
    //Ưu tiên xuất quân, ưu tiên Đá, chơi An toàn, ưu tiên về đích
    vector<Horse> horses; //Danh sách 4 quân cờ của người chơi
    vector<double> trongso; //5 Trọng số để Bot đánh giá nước đi
    
    Player(int ID, vector<double> ts, int t = -1); //Constructor 
    bool allFinished(); //Dùng để kiểm tra xem 4 quân cờ đã về đích hay chưa
    Move chonNuocdi(vector<Move>& moves); //Tự động chọn nước đi tốt nhất
    double tinhdiem(const Move& m); //Hàm tính điểm cho một nước đi
};

//Class đại diện cho bàn cờ
class Board {
public:
    vector<int> o;  //Mảng 48 phần tử đại diện cho các ô
    Board(); //Constructor
    vector<Move> getValidMoves(Player& p, const vector<Player>& allPlayers, int dice);
    //=> Lấy danh sách các nước đi hợp lệ
    void applyMove(Player& p, Move m, vector<Player>& players); //Thực hiện một nước đi
};

//Class xúc xắc
class Dice {
public:
    int roll();  //Đổ xúc xắc
};

//Class quản lí một ván chơi
class Game { 
public:
    vector<Player> players; //Danh sách người chơi
    Board board; //bàn cờ
    Dice dice; //Xúc xắc
    
    Game(vector<vector<double>> all_ts, vector<int> types); 
    int play1game(); //Mô phỏng chạy 1 ván 
};

#endif