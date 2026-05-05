#include "Game.h"

//Khởi tạo trạng thái ban đầu của cờ cá ngựa
Horse::Horse() {
    position = -1; //Chưa có vị trí trên bàn cờ
    state = IN_BASE; //Đang ở trong chuồng
    soBuocdi = 0; //Chưa đi bước nào
}

//Khởi tạo trạng thái của người chơi
Player::Player(int ID, vector<double> ts, int t) {
    id = ID;
    trongso = ts;
    type = t;
    horses = vector<Horse>(4); //Danh sách 4 quân cờ cho người chơi
}

//Kiểm tra điều kiện Thắng
bool Player::allFinished() {
    for (auto &h : horses) { // Dùng vòng lặp for-each tham chiếu duyệt qua 4 quân cờ
        if (h.state != FINISHED) return false;
        //=> Chỉ cần có 1 quân chưa về đích => Chưa thắng
    }
    return true; //Tất cả đều Finished thì Thắng
}

//Đánh giá nước đi
double Player::tinhdiem(const Move& m) {
    if (m.horseIndex == -1) return -1e18; //Nếu là nước đi rỗng (không đi được)
    //=> Trả về một số cực kì thấp
    
    vector<double> current_ts = trongso; //Sao chép bộ trọng số hiện tại

    //Logic cho hành vi của con Bot Ưu tiên lên Chuồng/Về đích
    if (type == 3) {
        double max_val = 0;
        //Tìm mức độ ưu tiên cao nhất trong các bộ trọng số
        for(double v : current_ts) if(v > max_val) max_val = v;
        
        //Reset toàn bộ trọng số về mức thấp
        for(int i=0; i<5; i++) current_ts[i] = 0.05;

        //Ép cho con Bot phải ưu tiên tuyệt đối việc đề đích nếu có thể
        if (m.is_finish) {
            current_ts[2] = max_val; // Gán trọng số cao nhất cho Về đích
        } else {
            current_ts[4] = max_val; //Nếu không thể về đích ngay
            //=> Ưu tiên các nước đi bình thường để lại gần đích hơn
        }
    }

    double diem = 0; 
    //Cộng điểm tương ứng nếu nước đi thỏa mãn các tính chất (nhân với trọng s6o1 của tính chất đó)
    diem += current_ts[0] * (m.is_raquan ? 1.0 : 0.0); //Cộng cho Ra quân
    diem += current_ts[1] * (m.is_kill ? 1.0 : 0.0);  //Cộng cho Đá
    diem += current_ts[2] * (m.is_finish ? 1.0 : 0.0);  //Cộng cho Về đích
    diem += current_ts[3] * (m.is_safe ? 1.0 : 0.0);  //Cộng cho đứng ở nơi An toàn
    //Tính điểm dựa trên quãng đường về đích(Càng về đích thì điểm càng cao hơn)
    //48 có nghĩa là phải đi qua 48 ô
    diem += current_ts[4] * (double)(48 - m.khoangcach_toidich) / 48.0;  
    
    return diem; //Trả về điểm sau khi đánh giá
}

//Duyệt qua danh sách các nước đi hợp lệ và chọn ra nước đi có điểm cao nhất
Move Player::chonNuocdi(vector<Move>& moves) {
    //Trả về nước đi rỗng (Bỏ lượt) để tránh cho trường hợp bị lỗi khi không thấy nước đi
    if (moves.empty()) return {-1, -1, false, false, false, false, 0};
    Move best = moves[0]; //Giả sử nước đi đầu tiên là tốt nhất
    double bestdiem = -1e18; //Điểm kỉ lục ban đầu cực thấp
    for (auto &m : moves) { 
        double diem = tinhdiem(m); //Tính điểm cho từng nước đi
        if (diem > bestdiem) {
            bestdiem = diem;
            best = m; //Cập nhật lại kỉ lục nếu tìm thấy nước đi ngon hơn
        }
    }
    return best; //Trả về quyết định cuối cùng sau khi đã xem xét các nước đi
} 

//Khởi tạo bàn cờ
Board::Board() { o = vector<int>(48, -1); }

//Tìm tất cả các nước đi khả thi trong một lượt tung xúc xắc
vector<Move> Board::getValidMoves(Player& p, const vector<Player>& allPlayers, int dice) {
    vector<Move> moves;
    for (int i = 0; i < 4; i++) { //Kiểm tra lần lượt 4 quân cờ
        Horse &h = p.horses[i];
        // Trường hợp 1: Quân cờ đang ở chuồng VÀ đổ được xúc xắc 6
        if (h.state == IN_BASE && dice == 6) {
            int startPos = (p.id * 12) % 48; // Tính ô xuất phát tùy thuộc ID người chơi (mỗi người cách nhau 12 ô)
            // Nếu ô xuất phát trống hoặc có quân địch đứng (không có quân mình)
            if (o[startPos] == -1 || o[startPos] != p.id) {
                bool kill = (o[startPos] != -1 && o[startPos] != p.id); //Xem có đá được quân địch ngay không
                moves.push_back({i, startPos, true, kill, false, true, 48});
                //=> Lưu nước đi này vào danh sách các nước đi khả thi
            }
        }
        // Trường hợp 2: Quân cờ đang di chuyển trên bàn cờ
        if (h.state == ON_BOARD) {
            int buocmoi = h.soBuocdi + dice; // Tính tổng số bước đi thêm
            if (buocmoi >= 48) moves.push_back({i, -1, false, false, true, true, 0});
            //Nếu như đã đi đủ vòng thì có thể về đích
            else {
                // Tính vị trí thực tế trên bàn cờ mảng vòng (0 đến 47)
                int vitrimoi = (h.position + dice) % 48;
                // Nếu ô đến không có quân phe mình chắn đường
                if (o[vitrimoi] != p.id) {
                    bool kill = (o[vitrimoi] != -1); //Nếu có quân địch ở đó => Đá
                    bool safe = true; //Mặc định là đang an toàn
                    //Kiểm tra độ an toàn bằng cách duyệt qua tất cả các người chơi
                    for (const auto& op : allPlayers) {
                        if (op.id == p.id) continue; //Bỏ qua bản thân
                        for (const auto& oh : op.horses) {
                            if (oh.state == ON_BOARD) {
                                //Tính khoảng cách từ địch tới vị trí của mình
                                int dist = (vitrimoi - oh.position + 48) % 48;
                                if (dist > 0 && dist <= 6) safe = false;
                                //=> Có nguy cơ bị đá
                            }
                        }
                    }
                    moves.push_back({i, vitrimoi, false, kill, false, safe, 48 - buocmoi});
                }
            }
        }
    }
    if (moves.empty()) moves.push_back({-1, -1, false, false, false, false, 0});
    // Nếu không có quân nào đi được, bắt buộc thêm 1 nước đi "bỏ lượt"
    return moves;
}

// Cập nhật trạng thái sau khi người chơi chốt nước đi
void Board::applyMove(Player &p, Move m, vector<Player>& players) {
    if (m.horseIndex == -1) return; //Nếu là nước đi Bỏ lượt thì bỏ qua luôn
    Horse &h = p.horses[m.horseIndex]; //Lấy quân cờ đã chọn
    if (h.state == ON_BOARD) o[h.position] = -1; //Xóa vị trí cũ
    //Nếu Ra quân
    if (m.is_raquan) { h.state = ON_BOARD; h.position = m.targetPos; h.soBuocdi = 0; }
    //Nếu về đích
    else if (m.is_finish) { h.state = FINISHED; h.position = -1; }
    else {
        int diff = (m.targetPos - h.position + 48) % 48; //Tính số bước vừa đi
        h.position = m.targetPos; //Cập nhật lại vị trí mới
        h.soBuocdi += diff; //Cộng dồn quãng đường đã đi
    }
    //Nếu nước đi này đá quân đói phương
    if (m.is_kill) {
        for (auto &other : players) {
            if (other.id == p.id) continue;
            for (auto &oh : other.horses) {
                //Tìm kẻ địch đang đứng tại ô mình vừa nhảy vào
                if (oh.state == ON_BOARD && oh.position == h.position) {
                    oh.state = IN_BASE; oh.position = -1; oh.soBuocdi = 0;
                }
            }
        }
    }
    if (h.state == ON_BOARD) o[h.position] = p.id;
    //=> Đánh dấu rằng ô hiện tại thuộc quyền sở hữu của người chơi này
}

//Đổ xúc xắc ngẫu nhiên từ 1 đến 6
int Dice::roll() { return rand() % 6 + 1; }

//Thiết lập một ván chơi mới với trọng số và chiến thuật cho 4 người
Game::Game(vector<vector<double>> all_ts, vector<int> types) {
    for (int i = 0; i < (int)all_ts.size(); i++) players.push_back(Player(i, all_ts[i], types[i]));
}

//Chạy mô phỏng 1 ván
int Game::play1game() {
    board = Board(); //Reset bàn cờ
    for (auto& p : players) for (auto& h : p.horses) h = Horse(); //Reset toàn bộ quân cờ
    vector<int> rank; //Lưu thứ tự về đích
    vector<bool> hasRank(4, false); //Đánh dấu xem người chơi đã được xếp hạng hay chưa
    int turnLimit = 1500; //Giới hạn 1500 lượt đấu
    while (rank.size() < 4 && turnLimit--) {
        for (auto &p : players) {
            if (p.allFinished()) continue; //Ai về đích thì không xét nữa
            int d = dice.roll(); //Tung xúc xắc
            vector<Move> moves = board.getValidMoves(p, players, d); //Lấy các nước đi hợp lệ
            Move chon = p.chonNuocdi(moves); //Suy nghĩ nước đi
            board.applyMove(p, chon, players);  //Thực hiện thay đổi trạng thái
            //Cập nhật bảng xếp hạng nếu có người chơi vừa cho quân cuối c2ung về đích
            if (p.allFinished() && !hasRank[p.id]) {
                rank.push_back(p.id); hasRank[p.id] = true;
            }
        }
    }
    return rank.empty() ? -1 : rank[0]; 
    //Trả về ID của người về nhất, nếu hòa (hết 1500 lượt) th2i trả về -1.
}