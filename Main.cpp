#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <random>
#include <memory>
#include <chrono>
#include <cmath>
#include <cassert>
#define repeat(i,n) for (int i = 0; (i) < (n); ++(i))
#define repeat_from(i,m,n) for (int i = (m); (i) < (n); ++(i))
#define repeat_reverse(i,n) for (int i = (n)-1; (i) >= 0; --(i))
#define repeat_from_reverse(i,m,n) for (int i = (n)-1; (i) >= (m); --(i))
#define whole(f,x,...) ([&](decltype((x)) whole) { return (f)(begin(whole), end(whole), ## __VA_ARGS__); })(x)
typedef long long ll;
using namespace std;
template <class T> void setmax(T & a, T const & b) { if (a < b) a = b; }
template <class T> void setmin(T & a, T const & b) { if (b < a) a = b; }
bool is_on_field(int y, int x, int h, int w) { return 0 <= y and y < h and 0 <= x and x < w; }

struct point_t { int y, x; };
point_t point(int y, int x) { return (point_t) { y, x }; }
point_t operator + (point_t const & a, point_t const & b) { return point(a.y + b.y, a.x + b.x); }
point_t operator - (point_t const & a, point_t const & b) { return point(a.y - b.y, a.x - b.x); }
point_t operator * (int a, point_t const & b) { return point(a * b.y, a * b.x); }
point_t & operator += (point_t & a, point_t const & b) { a.y += b.y; a.x += b.x; return a; }
point_t & operator -= (point_t & a, point_t const & b) { a.y -= b.y; a.x -= b.x; return a; }
template <typename T> point_t point(T const & p) { return (point_t) { p.y, p.x }; }
bool operator == (point_t a, point_t b) { return make_pair(a.y, a.x) == make_pair(b.y, b.x); }
bool operator != (point_t a, point_t b) { return make_pair(a.y, a.x) != make_pair(b.y, b.x); }
bool operator <  (point_t a, point_t b) { return make_pair(a.y, a.x) <  make_pair(b.y, b.x); }

template <typename T>
using functional_priority_queue = priority_queue<T, vector<T>, function<bool (T const &, T const &)> >;

struct clock_check {
    chrono::high_resolution_clock::time_point clock_end;
    clock_check(ll msec) {
        chrono::high_resolution_clock::time_point clock_begin = chrono::high_resolution_clock::now();
        clock_end = clock_begin + chrono::duration<ll, milli>(msec);
    }
    bool operator() () const {
        chrono::high_resolution_clock::time_point clock_current = chrono::high_resolution_clock::now();
        return clock_current < clock_end;
    }
};

struct stopwatch {
    chrono::high_resolution_clock::time_point clock_begin;
    stopwatch() {
        clock_begin = chrono::high_resolution_clock::now();
    }
    ll operator() () const {
        chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(clock_end - clock_begin).count();
    }
};

namespace primitive {
    const int pack_size = 3;
    const int width = 10;
    const int height = 16;
    const int erasing_sum = 10;
    const int turn_number = 500;

    typedef char block_t;
    const block_t empty_block = 0;
    const block_t obstacle_block = erasing_sum + 1;
    template <size_t H, size_t W>
    struct blocks_t {
        array<array<block_t, H>, W> at; // 左下が原点, x座標が先
    };
    typedef blocks_t<pack_size, pack_size> pack_t;
    typedef blocks_t<height, width> field_t;
    template <size_t H, size_t W>
    istream & operator >> (istream & in, blocks_t<H, W> & a) {
        repeat_reverse (y, H) {
            repeat (x, W) {
                int c; in >> c; a.at[x][y] = c;
                assert ((a.at[x][y] == 0 and 0 == empty_block)
                        or (1 <= a.at[x][y] and a.at[x][y] <= 9)
                        or (a.at[x][y] == 11 and 11 == obstacle_block));
            }
        }
        string s; in >> s; assert (s == "END");
        return in;
    }
    template <size_t H, size_t W>
    ostream & operator << (ostream & out, blocks_t<H, W> const & a) {
        repeat_reverse (y, H) {
            if (y != H-1) out << endl;
            repeat (x, W) {
                switch (a.at[x][y]) {
                    case empty_block: out << ' '; break;
                    case obstacle_block: out << '#'; break;
                    default: out << int(a.at[x][y]); break;
                }
            }
        }
        return out;
    }
    template <size_t H, size_t W> bool operator == (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at == b.at; }
    template <size_t H, size_t W> bool operator != (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at != b.at; }
    template <size_t H, size_t W> bool operator <  (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at <  b.at; }

    const int dangerline = height + 1;
    struct config_t {
        // int width, height;
        // int pack_size, erasing_sum;
        vector<pack_t> packs;
    };
    istream & operator >> (istream & in, config_t & a) {
        int w, h, t, s, n; in >> w >> h >> t >> s >> n;
        assert (w == width);
        assert (h == height);
        assert (t == pack_size);
        assert (s == erasing_sum);
        assert (n == turn_number);
        a.packs.resize(turn_number);
        repeat (i,n) in >> a.packs[i];
        return in;
    }

    struct input_t {
        int turn;
        int remaining_time; // in msec
        int self_obstacles;
        field_t self_field;
        int opponent_obstacles;
        field_t opponent_field;
    };
    istream & operator >> (istream & in, input_t & a) {
        in >> a.turn >> a.remaining_time;
        in >>     a.self_obstacles >>     a.self_field;
        in >> a.opponent_obstacles >> a.opponent_field;
        assert (a.self_obstacles == 0 or a.opponent_obstacles == 0);
        return in;
    }

    typedef int rotate_t;
    struct output_t {
        int x;
        rotate_t rotate;
    };
    ostream & operator << (ostream & out, output_t const & a) { return out << a.x << ' ' << a.rotate; }
    output_t make_output(int x, rotate_t rotate) { return { x, rotate }; }
    bool operator  < (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate)  < make_pair(b.x, b.rotate); }
    bool operator == (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate) == make_pair(b.x, b.rotate); }
    bool operator != (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate) != make_pair(b.x, b.rotate); }
    const output_t invalid_output = { (int)0xdeadbeef, -1 }; // 簡単のため範囲を拡張

    pack_t rotate(pack_t a, rotate_t r) {
        assert (pack_size == 3);
        pack_t b;
        switch (r % 4 + (r < 0 ? 4 : 0)) {
            case 0: b = a; break;
            case 1: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[2-y][  x]; break;
            case 2: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[2-x][2-y]; break;
            case 3: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[  y][2-x]; break;
        }
        return b;
    }
    pack_t fill_obstacles(pack_t a, int obstacles) {
        repeat_reverse (y, pack_size) { // yが先
            repeat (x, pack_size) {
                if (obstacles and a.at[x][y] == empty_block) {
                    a.at[x][y] = obstacle_block;
                    obstacles -= 1;
                }
            }
        }
        return a;
    }
    int count_empty_blocks(pack_t const & a) {
        int cnt = 0;
        repeat (x, pack_size) repeat (y, pack_size) if (a.at[x][y] == empty_block) ++ cnt;
        return cnt;
    }
    int count_consumed_obstacles(pack_t const & pack, int obstacles) {
        return max(0, min(count_empty_blocks(pack), obstacles));
    }

    const int chain_coefficient[] = {
        0,
        1, // 1
        1, // 2
        2, // 3
        2, // 4
        3, // 5
        4, // 6
        6, // 7
        8, // 8
        10, // 9
        13, // 10
        17, // 11
        23, // 12
        30, // 13
        39, // 14
        51, // 15
        66, // 16
        86, 112, 146, 190, 247, 321, 417, 542, 705, 917, 1192, 1550, 2015, 2619, 3405, 4427, 5756, 7482, 9727, 12646, 16440, 21372, 27783, 36118, 46954, 61040, 79353, 103159, 134106, 174338, 226640, 294632, 383022, 497929, 647307, 841500, 1093950, 1422135, 1848776, 2403409, 3124432, 4061761, 5280290, 6864377, 8923690, 11600797, 15081036, 19605347, 25486951, 33133037, 43072948, 55994833, 72793283, 94631268, 123020648, 159926843, 207904896, 270276365, 351359275, 456767058, 593797175, 771936328,
        1003517226, // 79
        1304572395, // 80
        1695944113, // 81
    };
    int calculate_score(int chain, int erase_count) {
        return chain_coefficient[chain] * (erase_count / 2);
    }
    int count_obstacles_from_delta(int base, int delta) {
        return (base + delta) / 5 - base / 5;
    }

    bool is_valid_output(field_t const & field, pack_t const & a_pack, output_t const & output) {
        if (0 <= output.x and output.x + pack_size <= width) return true;
        pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
        repeat (dy, pack_size) repeat (dx, pack_size) if (pack.at[dx][dy] != empty_block) {
            int x = output.x + dx;
            if (x < 0 or width <= x) return false;
        }
        return true;
    }
}
using namespace primitive;

namespace simulation {
    template<size_t H, size_t W>
    array<int,W> make_height_map(blocks_t<H,W> const & field) {
        array<int,W> h;
        repeat (x,W) {
            int y = 0;
            while (y < H and field.at[x][y] != empty_block) ++ y;
            h[x] = y;
        }
        return h;
    }
    vector<point_t> drop_pack(blocks_t<height + pack_size, width> & field, array<int,width> & height_map, pack_t const & a_pack, output_t const & output) {
        vector<point_t> modified_blocks;
        pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
        repeat (dy, 3) repeat (dx, 3) {
            if (pack.at[dx][dy] != empty_block) {
                int nx = output.x + dx;
                assert (0 <= nx and nx < width);
                field.at[nx][height_map[nx]] = pack.at[dx][dy];
                if (field.at[nx][height_map[nx]] != obstacle_block) {
                    modified_blocks.push_back(point(height_map[nx], nx));
                }
                ++ height_map[nx];
            }
        }
        return modified_blocks;
    }
    template<size_t H, size_t W>
    vector<pair<point_t,int> > collect_erases(blocks_t<H,W> const & field, vector<point_t> const & modified_blocks) {
        static const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
        static const int dx[] = {  0,  1, 1, 1 };
        vector<pair<point_t,int> > erases;
        for (point_t p : modified_blocks) { // 変化したところだけ見る
            assert (field.at[p.x][p.y] != empty_block and field.at[p.x][p.y] != obstacle_block);
            repeat (j, 4) {
                int cnt = 1;
                int acc = field.at[p.x][p.y];
                int ly = p.y - dy[j];
                int lx = p.x - dx[j];
                for (; is_on_field(ly, lx, H, W); ++ cnt) {
                    block_t block = field.at[lx][ly];
                    if (block == empty_block) break;
                    if (acc + block > erasing_sum) break;
                    acc += block;
                    ly -= dy[j];
                    lx -= dx[j];
                }
                // しゃくとり法っぽく
                int ry = p.y + dy[j];
                int rx = p.x + dx[j];
                while (cnt --) {
                    while (is_on_field(ry, rx, H, W)) {
                        block_t block = field.at[rx][ry];
                        if (block == empty_block) break;
                        if (acc + block > erasing_sum) break;
                        acc += block;
                        ry += dy[j];
                        rx += dx[j];
                    }
                    ly += dy[j];
                    lx += dx[j];
                    if (acc == erasing_sum) erases.emplace_back(point(ly, lx), j);
                    acc -= field.at[lx][ly];
                }
            }
        }
        whole(sort, erases);
        erases.erase(whole(unique, erases), erases.end()); // 重複排除
        return erases;
    }
    template<size_t H, size_t W>
    pair<int, vector<point_t> > apply_erases(blocks_t<H,W> const & field, vector<pair<point_t,int> > const & erases) {
        static const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
        static const int dx[] = {  0,  1, 1, 1 };
        int erase_count = 0;
        vector<point_t> used;
        for (auto && it : erases) {
            point_t p; int j; tie(p, j) = it;
            int cnt = 0, acc = 0;
            for (; acc != erasing_sum; ++ cnt) {
                acc += field.at[p.x][p.y];
                used.push_back(p);
                p.y += dy[j];
                p.x += dx[j];
            }
            erase_count += cnt;
        }
        whole(sort, used);
        used.erase(whole(unique, used), used.end());
        return { erase_count, used };
    }
    template<size_t H, size_t W>
    vector<point_t> erase_blocks(blocks_t<H,W> & field, array<int,W> & height_map, vector<point_t> const & points_to_erase) {
        auto & at = field.at;
        array<int,W> old_height_map = height_map;
        for (point_t p : points_to_erase) {
            at[p.x][p.y] = empty_block;
            setmin(height_map[p.x], p.y);
        }
        vector<point_t> modified_blocks;
        repeat (x,W) {
            for (int y = height_map[x] + 1; y < old_height_map[x]; ++ y) {
                if (at[x][y] != empty_block) {
                    at[x][height_map[x]] = at[x][y];
                    if (at[x][height_map[x]] != obstacle_block) {
                        modified_blocks.push_back(point(height_map[x], x));
                    }
                    ++ height_map[x];
                    at[x][y] = empty_block;
                }
            }
        }
        return modified_blocks;
    }

    struct result_t {
        int score;
        int chain;
    };
    bool compare_result_with_score(result_t const & a, result_t const & b) { return make_pair(a.score, a.chain) < make_pair(b.score, b.chain); }
    bool compare_result_with_chain(result_t const & a, result_t const & b) { return make_pair(a.chain, a.score) < make_pair(b.chain, b.score); }
    bool compare_result_with_score_reversed(result_t const & a, result_t const & b) { return make_pair(a.score, a.chain) > make_pair(b.score, b.chain); }
    bool compare_result_with_chain_reversed(result_t const & a, result_t const & b) { return make_pair(a.chain, a.score) > make_pair(b.chain, b.score); }
    bool operator < (result_t const & a, result_t const & b) { return compare_result_with_score(a, b); }

    template<size_t H, size_t W>
    result_t simulate(blocks_t<H,W> & field, array<int,W> & height_map, vector<point_t> modified_blocks, int initial_chain) {
        // 2. ブロックの消滅&落下処理
        int score = 0;
        int chain = initial_chain;
        while (not modified_blocks.empty()) {
            // 検査
            vector<pair<point_t,int> > erases = collect_erases(field, modified_blocks);
            if (erases.empty()) break; // 消滅なし
            ++ chain; // ここでincrement
            // 消滅&落下
            int erase_count; tie(erase_count, modified_blocks) = apply_erases(field, erases);
            modified_blocks = erase_blocks(field, height_map, modified_blocks);
            score += calculate_score(chain, erase_count);
        }
        // 3. 4. 5. 6. お邪魔ブロック関連処理
        // nop
        // 7. ターン終了
        result_t result;
        result.score = score;
        result.chain = chain;
        return result;
    }
    struct simulate_invalid_output_exception {};
    struct simulate_gameover_exception {};
    pair<result_t, field_t> simulate_with_output(field_t const & field, pack_t const & pack, output_t const & output) { // throws exceptions
        if (not is_valid_output(field, pack, output)) throw simulate_invalid_output_exception();
        blocks_t<height + pack_size, width> workspace;
        repeat (x, width) repeat (y,    height) workspace.at[x][y] = field.at[x][y];
        repeat (x, width) repeat (y, pack_size) workspace.at[x][height + y] = empty_block;
        // 1. パックの投下
        array<int,width> height_map = make_height_map(field);
        vector<point_t> modified_blocks = drop_pack(workspace, height_map, pack, output);
        // simulate()
        result_t result = simulate(workspace, height_map, modified_blocks, 0);
        // result
        repeat (x, width) if (height_map[x] > height) throw simulate_gameover_exception();
        field_t nfield;
        repeat (x, width) repeat (y, height) nfield.at[x][y] = workspace.at[x][y];
        return { result, nfield };
    }

    vector<pair<result_t, int> > estimate_with_erasing_all(field_t const & field) {
        const array<int,width> height_map = make_height_map(field);
        vector<pair<result_t, int> > acc;
        repeat (x, width) {
            repeat_reverse (y, height_map[x]) {
                if (field.at[x][y] == empty_block or field.at[x][y] == obstacle_block) continue;
                bool erasable =
                    y+1 >= height
                    or (x-1 >= 0    and field.at[x-1][y+1] == empty_block)
                    or (                field.at[x  ][y+1] == empty_block)
                    or (x+1 < width and field.at[x+1][y+1] == empty_block);
                if (not erasable) break;
                field_t nfield = field;
                array<int,width> nheight_map = height_map;
                vector<point_t> modified_blocks { point(y, x) };
                modified_blocks = erase_blocks(nfield, nheight_map, modified_blocks);
                result_t result = simulate(nfield, nheight_map, modified_blocks, 1);
                int consumed = 0;
                repeat (x, width) consumed += height_map[x] - nheight_map[x];
                acc.emplace_back(result, consumed);
            }
        }
        if (acc.empty()) acc.emplace_back((result_t) { 0, 0 }, 0);
        return acc;
    }
    result_t estimate_with_erasing(field_t const & field) {
        vector<pair<result_t, int> > acc = estimate_with_erasing_all(field);
        return whole(max_element, acc, [&](pair<result_t, int> const & a, pair<result_t, int> const & b) {
            return make_tuple(a.first.chain, - a.second, a.first.score) < make_tuple(b.first.chain, - b.second, b.first.score);
        })->first;
    }

    result_t estimate_with_drop(field_t const & field) {
        const array<int,width> height_map = make_height_map(field);
        result_t acc = { -1, 0 };
        repeat (x, width) {
            repeat_from (b,1,9+1) {
                blocks_t<height + 1, width> nfield = {};
                repeat (nx, width) repeat (y, height_map[nx]) nfield.at[nx][y] = field.at[nx][y];
                nfield.at[x][height_map[x]] = b;
                array<int,width> nheight_map = height_map;
                nheight_map[x] += 1;
                vector<point_t> modified_blocks { point(height_map[x], x) };
                result_t result = simulate(nfield, nheight_map, modified_blocks, 0);
                setmax(acc, result);
            }
        }
        return acc;
    }
}
using namespace simulation;

const int summary_depth = 12;
struct state_summary_t {
    int base_turn;
    // そのターンのパックのみを落として検査する
    result_t result[summary_depth];
    result_t estimated[summary_depth + 1];
    result_t best_result[summary_depth]; // そのターン以降で
    result_t best_estimated[summary_depth + 1];
};
state_summary_t summarize_state(config_t const & config, int current_turn, field_t const & field, int a_obstacles) {
    state_summary_t info = {};
    info.base_turn = current_turn;
    info.estimated[0] = estimate_with_erasing(field);
    int obstacles = a_obstacles;
    repeat (age, summary_depth) {
        if (current_turn + age >= config.packs.size()) break;
        pack_t pack = fill_obstacles(config.packs[current_turn + age], min(9, obstacles));
        obstacles -= min(9, obstacles);
        repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
            result_t result; field_t nfield;
            try {
                tie(result, nfield) = simulate_with_output(field, pack, make_output(x, r));
            } catch (simulate_invalid_output_exception e) {
                continue;
            } catch (simulate_gameover_exception e) {
                continue;
            }
            setmax(info.result[age], result);
            setmax(info.estimated[age], estimate_with_erasing(nfield));
        }
    }
    info.best_result   [summary_depth  -1] = info.result   [summary_depth  -1];
    info.best_estimated[summary_depth+1-1] = info.estimated[summary_depth+1-1];
    repeat_reverse (age, summary_depth   - 1) info.best_result   [age] = max(info.best_result   [age+1], info.result   [age]);
    repeat_reverse (age, summary_depth+1 - 1) info.best_estimated[age] = max(info.best_estimated[age+1], info.estimated[age]);
    return info;
}

struct evaluateion_info_t {
    double score;
    double base_score;
    double permanent_bonus;
    bool is_effective_fired;
    result_t estimated;
};
struct photon_t {
    int turn;
    int score;
    int obstacles; // 負なら相手に送る分 / 相手が発火したらここが変わり次以降の状態が消去される
    field_t field;
    output_t output; // この手に至る辺
    result_t result; // その結果
    evaluateion_info_t evaluation;
    weak_ptr<photon_t> parent; // 逆辺
    shared_ptr<array<array<shared_ptr<photon_t>, 4>, 12> > next; // 次状態への辺
};

shared_ptr<photon_t> initial_photon(int turn, int obstacles, field_t const & field) {
    shared_ptr<photon_t> pho = make_shared<photon_t>();
    pho->turn = turn;
    pho->score = 0;
    pho->obstacles = obstacles;
    pho->field = field;
    pho->output = invalid_output;
    pho->result = {};
    pho->parent.reset();
    pho->next = nullptr;
    // evaluation
    pho->evaluation.score = 0;
    pho->evaluation.base_score = 0;
    pho->evaluation.permanent_bonus = 0;
    pho->evaluation.is_effective_fired = false;
    pho->evaluation.estimated = {};
    return pho;
}

void evaluate_photon_base(shared_ptr<photon_t> const & pho) {
    // 初期化
    pho->evaluation.score = 0;
    pho->evaluation.permanent_bonus = 0;
    pho->evaluation.is_effective_fired = false;
    // 評価
    pho->evaluation.estimated = estimate_with_erasing(pho->field);
    double acc = 0;
    acc += pho->score; // scoreを基準に
    acc += pho->evaluation.estimated.chain * 30; // 大連鎖だと4*5なんて誤差、小連鎖でscoreを気にされると不利
    acc -= 3 * max(0, min(160, pho->obstacles - pho->evaluation.estimated.score / 6));
    repeat (x, width) repeat (y, height) {
        int dx = min(x, width-x-1);
        if (pho->field.at[x][y] == obstacle_block) {
            acc -= 3 + 0.3 * y + 1.2 * dx; // 端に寄せた方がいいけど、あまりそればかり気にされても困る
        }
    }
    if (pho->result.chain == 1) acc -= 6.0 * pho->result.score; // 手数で損
    if (pho->result.chain == 2) acc -= 5.0 * pho->result.score;
    if (pho->result.chain == 3) acc -= 4.0 * pho->result.score;
    if (pho->result.chain == 4) acc -= 3.0 * pho->result.score;
    if (pho->result.chain == 5) acc -= 2.0 * pho->result.score;
    pho->evaluation.base_score = acc;
}

// 次のstepを(まだなら)作成 評価は半分しない
void step_photon(shared_ptr<photon_t> const & pho, pack_t const & pack) {
    if (pho->next) return;
    int consumed = count_consumed_obstacles(pack, pho->obstacles);
    pack_t filled_pack = fill_obstacles(pack, consumed);
    pho->next = make_unique<array<array<shared_ptr<photon_t>, 4>, 12> >();
    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
        shared_ptr<photon_t> npho = make_shared<photon_t>();
        npho->output = make_output(x, r);
        try {
            tie(npho->result, npho->field) = simulate_with_output(pho->field, filled_pack, npho->output);
        } catch (simulate_invalid_output_exception e) {
            continue;
        } catch (simulate_gameover_exception e) {
            continue;
        }
        npho->turn = pho->turn + 1;
        npho->score = pho->score + npho->result.score;
        npho->obstacles = pho->obstacles - consumed - count_obstacles_from_delta(pho->score, npho->result.score);
        npho->parent = pho;
        (*pho->next)[x+pack_size-1][r] = npho;
        evaluate_photon_base(npho);
    }
}

// 相手の発火でお邪魔が増えたとき
void update_photon_obstacles(shared_ptr<photon_t> const & pho, int updated_obstacles, vector<pack_t> const & packs) {
    if (pho->obstacles == updated_obstacles) {
        // nop
    } else if (not pho->next) {
        pho->obstacles = updated_obstacles;
    } else {
        int         consumed = count_consumed_obstacles(packs[pho->turn],    pho->obstacles);
        int updated_consumed = count_consumed_obstacles(packs[pho->turn], updated_obstacles);
        pho->obstacles = updated_obstacles;
        if (consumed != updated_consumed) {
            pho->next = nullptr; // 開放
        } else {
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                if (not npho) continue;
                int next_obstacles = pho->obstacles - updated_consumed - count_obstacles_from_delta(pho->score, npho->result.score);
                update_photon_obstacles(npho, next_obstacles, packs);
            }
        }
    }
}

const int chain_of_fire = 6; // 発火したとみなすべき連鎖数 (inclusive)
bool is_effective_firing(int score, int obstacles, int age, state_summary_t const & oppo_sum) {
    assert (4 < summary_depth);
    int i = min(4, age-1);
    assert (i >= 0);
    return max<int>(oppo_sum.best_result[i].score, oppo_sum.best_estimated[i].score * 0.8) + max(obstacles + 20, 60) * 5 < 1.2 * score;
}

void evaluate_photon_init(shared_ptr<photon_t> const & pho) {
    pho->evaluation.score = 0;
    pho->evaluation.permanent_bonus = 0;
    pho->evaluation.is_effective_fired = false;
}

// 相手の情報等を注入し評価を修正する
void evaluate_photon(shared_ptr<photon_t> const & pho, int base_turn, state_summary_t const & oppo_sum, int stress) {
    int age = pho->turn - base_turn;
    shared_ptr<photon_t> ppho = pho->parent.lock();
    pho->evaluation.is_effective_fired = is_effective_firing(pho->result.score, pho->obstacles, age, oppo_sum);
    {
        double acc = 0;
        if (ppho) acc += ppho->evaluation.permanent_bonus; // 前のやつに足していく
        if (pho->result.chain >= chain_of_fire) {
            // double k = max(pow(1 + 0.02 * stress, -age+1), 0.4 - 0.02 * age); // 弱いっぽい？
            double k = pow(1 + 0.01 * stress, -age+1);
            acc -= pho->result.score;
            if (pho->evaluation.is_effective_fired) {
                acc += (20 * stress) + k * (1.2 * pho->result.score); // 発火した それが可能でしかも勝てるというのは大きい
            } else {
                acc += k * pho->result.score;
            }
        }
        pho->evaluation.permanent_bonus = acc;
    }
    {
        double acc = 0;
        acc += pho->evaluation.base_score;
        acc += max(0.1, 1 - 0.03 * (age - 1)) * pho->evaluation.estimated.score; // 不正確な値だけど比較可能だろうからよい
        pho->evaluation.score = acc + pho->evaluation.permanent_bonus;
    }
}

shared_ptr<photon_t> parent_photon_at(shared_ptr<photon_t> pho, int turn) {
    while (pho and pho->turn > turn) pho = pho->parent.lock();
    if (not pho or pho->turn != turn) return nullptr;
    return pho;
}

void prune_photon(shared_ptr<photon_t> const & pho, output_t output) {
    assert (pho);
    if (not pho->next) return;
    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
        if (x == output.x and r == output.rotate) continue;
        (*pho->next)[x+pack_size-1][r] = nullptr;
    }
}

bool compare_photon(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->evaluation.score < b->evaluation.score; }
bool compare_photon_reversed(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->evaluation.score > b->evaluation.score; }
bool compare_photon_with_score(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->result.score < b->result.score; }
bool compare_photon_with_score_reversed(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->result.score > b->result.score; }
bool compare_photon_with_first(pair<double, weak_ptr<photon_t> > const & a, pair<double, weak_ptr<photon_t> > const & b) { return a.first < b.first; }

template <typename F>
void beam_search(shared_ptr<photon_t> const & initial, config_t const & config, int beam_width, int beam_depth, F cont) {
    vector<shared_ptr<photon_t> > beam, nbeam;
    beam.push_back(initial);
    repeat (i, beam_depth) {
        if (initial->turn + i >= config.packs.size()) break;
        for (shared_ptr<photon_t> const & pho : beam) {
            step_photon(pho, config.packs[initial->turn + i]);
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                if (not npho) continue;
                npho = cont(npho);
                if (not npho) continue;
                if (i+1 < beam_depth) {
                    nbeam.push_back(npho);
                }
            }
        }
        beam.resize(min<int>(beam_width, nbeam.size()));
        partial_sort_copy(nbeam.begin(), nbeam.end(), beam.begin(), beam.end(), compare_photon_reversed);
        nbeam.clear();
    }
}

template <typename F>
void chokudai_search(deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > & que, config_t const & config, int initial_turn, int beam_width, int beam_depth, ll time_limit_msec, F cont) {
    assert (initial_turn + beam_depth < config.packs.size());
    assert (beam_depth + 1 < que.size());
    clock_check check(time_limit_msec);
    while (check()) {
        repeat (i, beam_depth) {
            repeat (j, beam_width) {
                if (que[i].empty()) break;
                shared_ptr<photon_t> pho = que[i].top().second.lock(); que[i].pop();
                if (not pho) { -- j; continue; } // 無視して同じ深さをもう一度
                step_photon(pho, config.packs[initial_turn + i]);
                repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                    shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                    if (not npho) continue;
                    npho = cont(npho);
                    if (not npho) continue;
                    que[i+1].emplace(npho->evaluation.score, npho);
                }
            }
        }
    }
}

template <typename F>
void chokudai_search(shared_ptr<photon_t> const & initial, config_t const & config, int beam_width, int beam_depth, ll time_limit_msec, F cont) {
    deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > que;
    repeat (i, beam_depth + 1) que.emplace_back(compare_photon_with_first);
    que[0].emplace(initial->evaluation.score, initial);
    chokudai_search(que, config, initial->turn, beam_width, beam_depth, time_limit_msec, cont);
}

class AI {
private:
    config_t config;
    vector<input_t> inputs;
    vector<output_t> outputs;
    vector<int> scores;
    vector<result_t> results;

private:
    default_random_engine engine;
    double random() {
        uniform_real_distribution<double> dist;
        return dist(engine);
    }
    int randint(int l, int r) { // [l, r]
        uniform_int_distribution<int> dist(l, r);
        return dist(engine);
    }

private:
    static const int beam_small_width = 3;
    static const int beam_chain_max = 32;
    static constexpr double beam_output_limit_rate = 0.3;
    vector<shared_ptr<photon_t> > self_history;

public:
    AI(config_t const & a_config) {
        engine = default_random_engine(); // fixed seed
        config = a_config;
    }
    output_t think(input_t const & input) {
        stopwatch watch, total_watch;

        // logging
        cerr << endl;
        cerr << "turn: " << input.turn << endl;
        cerr << "remaining time: " << input.remaining_time << endl;
        cerr << "score: " << (scores.empty() ? 0 : scores.back()) << endl;
        cerr << "obstacles: " << input.self_obstacles - input.opponent_obstacles << endl;
        cerr << "self estimated chain: " << estimate_with_erasing(input.self_field).chain << endl;
        cerr << "opp. estimated chain: " << estimate_with_erasing(input.opponent_field).chain << endl;

        // check
        if (not inputs.empty()) {
            field_t const & field = input.self_field;
            auto & last = inputs.back();
            pack_t const & last_filled_pack = fill_obstacles(config.packs[last.turn], last.self_obstacles);
            result_t result; field_t nfield; tie(result, nfield) = simulate_with_output(last.self_field, last_filled_pack, outputs.back());
            if (nfield != field) {
                cerr << "<<<" << endl;
                cerr << nfield << endl;
                cerr << "===" << endl;
                cerr << field << endl;
                cerr << ">>>" << endl;
            }
            assert (nfield == field);
        }

        // prepare
        const state_summary_t oppo_sum = summarize_state(config, input.turn, input.opponent_field, input.opponent_obstacles);
        if (self_history.empty()) self_history.push_back(initial_photon(input.turn, input.self_obstacles, input.self_field));
        shared_ptr<photon_t> const & pho = self_history.back();
        update_photon_obstacles(pho, input.self_obstacles - input.opponent_obstacles, config.packs);
        evaluate_photon_init(pho);
        const int stress = max(pho->evaluation.estimated.chain, oppo_sum.best_estimated[0].chain);

        // opponent
        /* vector<int> opponent_score_at(8); { // 後で
            const int beam_width = 3;
            const int time_limit = 300; // msec
            auto cont = [&](shared_ptr<photon_t> const & pho) {
                evaluate_photon(pho, input.turn, oppo_sum, stress); // 評価は文脈を使うのでここでやる
                int age = pho->turn - input.turn;
                assert (age >= 1);
                setmax(opponent_score_at[age-1], pho->result.score);
                return pho->result.chain < chain_of_fire; // 打ち切るか否か
            };
            shared_ptr<photon_t> pho = initial_photon(input.turn, input.opponent_field_obstacles, input.opponent_field);
            chokudai_search(pho, config, input, beam_width, opponent_score_at.size(), time_limit, cont);
        } */

        // chokudai search
        watch = stopwatch();
        output_t output = invalid_output; {
            const int beam_width = 3;
            const int beam_depth = max(6, 18 - stress/4);
            const int time_limit = min(input.remaining_time / 30, max(800, 120 * stress)); // msec
            bool is_fired = false;
            double best_score = - INFINITY;
            shared_ptr<photon_t> result = nullptr;
            auto cont = [&](shared_ptr<photon_t> const & pho) {
                evaluate_photon(pho, input.turn, oppo_sum, stress); // 評価は文脈を使うのでここでやる
                int age = pho->turn - input.turn;
                assert (age >= 1);
                bool cur_is_fired = pho->evaluation.is_effective_fired;
                double cur_score = pho->evaluation.score;
                if (make_pair(is_fired, best_score) < make_pair(cur_is_fired, cur_score)) {
                    is_fired = cur_is_fired;
                    best_score = cur_score;
                    result = pho;
                    cerr << "result: +" << age << "t " << cur_score << "pt";
                    if (pho->result.chain >= chain_of_fire) {
                        bool is_eff = pho->evaluation.is_effective_fired;
                        cerr << " (fire " << pho->result.chain << "c " << pho->result.score << "pt" << (is_eff ? " eff." : "") << ")";
                    } else {
                        cerr << " (" << pho->evaluation.estimated.chain << "c " << pho->evaluation.estimated.score << "pt)";
                    }
                    cerr << endl;
                }
                if (pho->result.chain < chain_of_fire) { // 打ち切るか否か
                    return pho;
                } else {
                    return shared_ptr<photon_t>(nullptr);
                }
            };
            cerr << "chokudai search" << endl;
            chokudai_search(pho, config, beam_width, beam_depth, time_limit, cont);
            if (result) {
                shared_ptr<photon_t> pho = parent_photon_at(result, input.turn + 1);
                assert (pho);
                output = pho->output;
            }
            cerr << "done" << endl;
        }
        cerr << "self elapsed: " << watch() << "ms" << endl;

        // finalize
        const pack_t filled_pack = fill_obstacles(config.packs[input.turn], input.self_obstacles);
        if (output == invalid_output) {
            cerr << "search failed..." << endl;
            int score = -1;
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                try {
                    result_t result = simulate_with_output(input.self_field, filled_pack, make_output(x, r)).first;
                    if (score <= result.score) {
                        score = result.score;
                        output = make_output(x, r);
                    }
                } catch (simulate_invalid_output_exception e) {
                    // nop
                } catch (simulate_gameover_exception e) {
                    // nop
                }
            }
            if (output == invalid_output) {
                cerr << "greedy failed..." << endl;
            }
        }
        if (is_valid_output(input.self_field, filled_pack, output)) {
            inputs.push_back(input);
            outputs.push_back(output);
            results.push_back(simulate_with_output(input.self_field, filled_pack, output).first);
            int last_score = scores.empty() ? 0 : scores.back();
            scores.push_back(last_score + results.back().score);
            prune_photon(self_history.back(), output);
            self_history.push_back((*self_history.back()->next)[output.x+pack_size-1][output.rotate]);
        }
        cerr << "total elapsed: " << total_watch() << "ms" << endl;
        return output;
    }
};

const string ai_name = "SampleAI.cpp";
int main() {
    cout << ai_name << endl;
    cout.flush();
    config_t config; cin >> config;
    AI ai(config);
    repeat (i, config.packs.size()) {
        input_t input; cin >> input;
        if (not cin) { cerr << "error: input failed" << endl; break; }
        output_t output = ai.think(input);
        cout << output << endl;
        cout.flush();
    }
    return 0;
}
