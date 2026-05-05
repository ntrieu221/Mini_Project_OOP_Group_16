#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include "Game.h"

using namespace std;

/**
 * Hàm tạo bộ trọng số ngẫu nhiên cho Bot.
 * Quy tắc: Tổng các trọng số = 1.0, mỗi bước nhảy là 0.1.
 * @param type_idx: Chỉ số hành động đặc trưng của loại Bot đó (sẽ được ưu tiên cao nhất).
 */
vector<double> generateWeights(int type_idx) {
    // Khởi tạo vector 5 phần tử, mỗi phần tử tối thiểu là 1 (tương đương 0.1)
    vector<int> v(5, 1); 
    int remaining = 5;// Còn lại 5 phần (0.5) để phân bổ ngẫu nhiên
    
    // Phân phối ngẫu nhiên số điểm còn lại vào 5 ô trọng số
    while(remaining > 0) {
        v[rand() % 5]++;
        remaining--;
    }

    // Tìm xem trong bộ vừa tạo, ô nào đang có giá trị lớn nhất
    int current_max_idx = 0;
    for(int i = 1; i < 5; i++) {
        if(v[i] > v[current_max_idx]) current_max_idx = i;
    }

    // Hoán vị để đảm bảo (type_idx) luôn giữ giá trị lớn nhất
    swap(v[type_idx], v[current_max_idx]);

    // Chuyển đổi từ số nguyên sang số thực (ví dụ: 3 -> 0.3)
    vector<double> ts(5);
    for (int i = 0; i < 5; i++) ts[i] = v[i] / 10.0;
    return ts;
}

/**
 * Hàm tìm kiếm bộ trọng số tối ưu nhất cho một loại Bot cụ thể.
 * Tạo ra một nhóm (pool) gồm 20 bộ trọng số, cho chúng đấu với nhau để chọn ra bộ thắng nhiều nhất.
 */
vector<double> findBestForType(int behavior_type, int weight_idx, string label) {
    vector<vector<double>> pool;
    // Tạo 20 bộ trọng số khác nhau
    for(int i=0; i<20; i++) pool.push_back(generateWeights(weight_idx));

    vector<int> wins(20, 0); // Lưu số trận thắng của từng bộ
    int matches = 200;       // Tổng số trận đấu thử nghiệm

    for(int i=0; i<matches; i++) {
        // Chọn ngẫu nhiên 4 bộ từ 20 người để thi đấu một trận
        vector<int> contestants;
        while(contestants.size() < 4) {
            int r = rand() % 20;
            // Đảm bảo không chọn trùng một bộ vào cùng một trận
            if(find(contestants.begin(), contestants.end(), r) == contestants.end())
                contestants.push_back(r);
        }

        vector<vector<double>> match_ts;
        vector<int> match_types(4, behavior_type);
        for(int idx : contestants) match_ts.push_back(pool[idx]);

        // Khởi tạo trò chơi và lấy ID người thắng cuộc
        Game game(match_ts, match_types);
        int win_id = game.play1game();
        if(win_id != -1) wins[contestants[win_id]]++; // Cộng điểm thắng cho ứng viên đó
    }

    // Tìm ứng viên có số trận thắng cao nhất trong phép thử
    int best = 0;
    for(int i=1; i<20; i++) if(wins[i] > wins[best]) best = i;
    return pool[best];
}

int main() {
    // Khởi tạo giá trị ngẫu nhiên
    srand(time(0));
    cout << fixed << setprecision(1);
    vector<double> ts_spawn    = findBestForType(0, 0, "Bot Xuat Quan");
    vector<double> ts_aggro    = findBestForType(1, 1, "Bot Hung Hang (Da)");
    vector<double> ts_evasive  = findBestForType(2, 3, "Bot Ne (Safe)");
    vector<double> ts_finisher = findBestForType(3, 2, "Bot Len Chuong");

    //Tổ chức cho các chiến thuật đấu với nhau
    vector<vector<double>> finalists = {ts_spawn, ts_aggro, ts_evasive, ts_finisher};
    vector<int> types = {0, 1, 2, 3}; 
    vector<int> total_wins(4, 0);
    int final_rounds = 100; // Chạy 100 trận để lấy thống kê % thắng

    for(int i=0; i<final_rounds; i++) {
        Game game(finalists, types);
        int winner = game.play1game();
        if(winner != -1) total_wins[winner]++;
    }

    //Hiển thị kết quả
    string names[] = {"Bot Xuat Quan", "Bot Hung Hang", "Bot Ne (Safe)", "Bot Len Chuong"};
    string labels[] = {"Xuat quan", "Da doi thu", "Ve dich", "Ne (Safe)", "Di binh thuong"};

    // Xác định con Bot nào vô địch dựa trên số trận thắng
    int champion = 0;
    for(int i=1; i<4; i++) if(total_wins[i] > total_wins[champion]) champion = i;

    // In bảng kết quả
    cout << "100 TRAN DAU GIUA 4 BOT\n";
    for(int i=0; i<4; i++) {
        cout << left << setw(19) << names[i] << ": " << total_wins[i] 
             << " win (" << (double)total_wins[i] << "%)\n";
    }
    
    // In thông tin về Bot vô địch và bộ trọng số tối ưu của nó
    cout << "\nLOAI BOT VO DICH: " << names[champion] << "\n";
    cout << "Bo trong so toi uu nhat tim duoc:\n";
    for(int i=0; i<5; i++) {
        //Căn lề
        cout << "- " << left << setw(17) << labels[i] << ": " << finalists[champion][i] << endl;
    }

    return 0;
}