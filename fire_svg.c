#include <stdio.h>
#include <math.h>
#include "config.h"

typedef struct {
    double lambda;   // 基本波長(px)
    double amp;      // 基本振幅(px)
    double y;        // ベース高さ
    double speed;    // 横移動の速度(px/sec) → dur = 距離 / 速度
    double wobbleA;  // 垂直ゆらぎの振幅(px)
    double wobbleT;  // 垂直ゆらぎの周期(sec)
    const char* fill;// 塗り色
    double opacity;  // 不透明度
} Layer;

static void wave_path(char *out, size_t cap, double W, double H, double x_start, double lambda, double A, double y, int tiles) {
    // 合成波 y + A*sin(2πx/λ) + 0.35A*sin(4πx/λ + φ)
    // をベジェ列で近似。区間分割数を上げるとより自然。
    int segs = 16;                  // 1λをいくつに割るか（多いほど滑らか）
    double phi = 1.3;               // 第2高調波の位相
    double L = lambda * tiles;      // 全体長（生成する横幅）
    double x_start_local = x_start; // 開始x（負にして左右の見切れを防ぐ）
    int pos = 0;
    pos += snprintf(out+pos, cap-pos, "M %.2f %.2f ", x_start_local, y);

    for (int t = 1; t <= segs * tiles; ++t) {
        double x0 = x_start_local + (double)(t-1) * (L/(segs*tiles));
        double x1 = x_start_local + (double)t * (L/(segs*tiles));

        double yy0 = y
            + A * sin(2.0*M_PI*x0/lambda)
            + 0.35*A * sin(4.0*M_PI*x0/lambda + phi);
        double yy1 = y
            + A * sin(2.0*M_PI*x1/lambda)
            + 0.35*A * sin(4.0*M_PI*x1/lambda + phi);

        // 接線勾配（導関数）から制御点を簡易構築
        double dy0 = (A*(2.0*M_PI/lambda)*cos(2.0*M_PI*x0/lambda))
                   + (0.35*A*(4.0*M_PI/lambda)*cos(4.0*M_PI*x0/lambda + phi));
        double dy1 = (A*(2.0*M_PI/lambda)*cos(2.0*M_PI*x1/lambda))
                   + (0.35*A*(4.0*M_PI/lambda)*cos(4.0*M_PI*x1/lambda + phi));
        // Catmull-Rom風に、x方向一定幅でCベジェ制御点
        double dx = (x1 - x0) / 3.0;
        double c1x = x0 + dx, c1y = yy0 + dy0 * dx;
        double c2x = x1 - dx, c2y = yy1 - dy1 * dx;

        pos += snprintf(out+pos, cap-pos, "C %.2f %.2f, %.2f %.2f, %.2f %.2f ",
                        c1x, c1y, c2x, c2y, x1, yy1);
        if (pos >= (int)cap-200) break;
    }

    // 下辺に落として塗り潰す
    double x_end = x_start_local + L;
    pos += snprintf(out+pos, cap-pos, "L %.2f %.2f L %.2f %.2f Z", x_end, H, x_start_local, H);
}

static void falling_embers(FILE *fp, double W, double H) {
    fprintf(fp, "<g id='embers' opacity='0.95'>\n");
    unsigned int seed = 987654u;
    #define RND() (seed = 1664525u * seed + 1013904223u, (seed>>8) / (double)0xFFFFFFu)
    for (int i = 0; i < 500; i++) {                 // 粒の数は控えめ（背景用）
        double x = W * RND();
        double y0 = -H * 0.25 * RND();              // 画面上の外から降り始め
        double drift = (RND() - 0.5) * 36.0;        // 横ズレ
        double fall = H + H * 0.35 * RND();         // 落下距離（下に抜ける）
        double r = 1.5 + 1.6 * RND();               // 粒半径
        double delay = 30.0 * RND();                // ランダム開始
        double dur = 4.5 + 3.0 * RND();             // 落下時間

        fprintf(fp,
          "<g>\n"
          "  <circle cx='%.1f' cy='%.1f' r='%.2f' fill='url(#emberGrad)' opacity='1.0'/>\n"
          "  <animateTransform attributeName='transform' type='translate' additive='sum'\n"
          "    from='0 0' to='%.1f %.1f' dur='%.2fs' begin='%.2fs' repeatCount='indefinite'/>\n"
          "  <animate attributeName='opacity' values='0;1;0' dur='%.2fs' begin='%.2fs' repeatCount='indefinite'/>\n"
          "</g>\n",
          x, y0, r,
          drift, fall, dur, delay,
          dur, delay
        );
    }
    #undef RND
    fprintf(fp, "</g>\n");
}

// --- 頂上から立ち上る煙（アニメーション）を描くユーティリティ ---
static void draw_rising_smoke(FILE *fp, double cx, double topY, double H, double topWidth) {
    fprintf(fp, "  <g id='smokeStack'>\n");
    for (int i = 0; i < 12; ++i) {
        double delay = 0.6 * i;                 // 時間差
        double dx = ((i % 2) ? -1.0 : 1.0) * (6.0 + 2.0 * (i % 3)); // 左右に微ドリフト
        double up = H * 0.35 + 6.0 * (i % 5);   // 上昇距離
        double rx0 = 12.0, ry0 = 6.0;            // 初期楕円サイズ
        double rx1 = 50.0, ry1 = 30.0;          // 拡散後サイズ

        fprintf(fp, "    <g transform='translate(%.1f %.1f)' opacity='0.55' visibility='hidden'>\n", cx, topY - 2.0);
        fprintf(fp, "      <set attributeName='visibility' to='visible' begin='%.1fs' dur='0.0001s' fill='freeze'/>\n", delay);
        fprintf(fp,
                "      <ellipse cx='0' cy='0' rx='%.1f' ry='%.1f' fill='rgba(100,100,100,0.55)' opacity='0' filter='url(#smokeBlur)'>\n"
                "        <animateTransform attributeName='transform' type='translate' additive='sum' from='0 0' to='%.1f -%.1f' dur='5s' begin='%.1fs' repeatCount='indefinite'/>\n"
                "        <animate attributeName='rx' values='%.1f;%.1f' dur='5s' begin='%.1fs' repeatCount='indefinite'/>\n"
                "        <animate attributeName='ry' values='%.1f;%.1f' dur='5s' begin='%.1fs' repeatCount='indefinite'/>\n"
                "        <animate attributeName='opacity' values='0.1;0.8;0.4;0' dur='5s' begin='%.1fs' repeatCount='indefinite'/>\n"
                "      </ellipse>\n",
                rx0, ry0,
                dx, up, delay,
                rx0, rx1, delay,
                ry0, ry1, delay,
                delay);
        fprintf(fp, "    </g>\n");
    }
    fprintf(fp, "  </g>\n");
}

int main(void) {
    const double W = 1300, H = 490;

    Layer L[] = {
        //  溶岩レイヤー:   λ     A      y         speed  wobA wobT   color        opac
        {  520,   14,  H*0.72,    14,   1.8, 28.0, "#3b0d0d", 0.50 }, // 奥（遅い・浅い色）
        {  640,   18,  H*0.76,    18,   2.2, 32.0, "#551312", 0.52 },
        {  760,   22,  H*0.80,    22,   2.8, 36.0, "#7a1c11", 0.54 },
        {  900,   28,  H*0.83,    28,   3.4, 40.0, "#b62e0d", 0.56 },
        { 1100,   36,  H*0.86,    36,   4.2, 44.0, "#ff5c1a", 0.58 }  // 手前（やや速い・明るい）
    };
    const int layers = (int)(sizeof(L)/sizeof(L[0]));

    // SVG ヘッダ + 背景用 <defs>
    printf(
      "<svg xmlns='http://www.w3.org/2000/svg' width='%.0f' height='%.0f' viewBox='0 0 %.0f %.0f'>\n"
      "<defs>\n"
      // 空（赤黒の火山風グラデ）
      "  <linearGradient id='sky' x1='0' y1='0' x2='0' y2='1'>\n"
      "    <stop offset='0%%'  stop-color='#1a0e0a'/>\n"
      "    <stop offset='60%%' stop-color='#2b100b'/>\n"
      "    <stop offset='100%%' stop-color='#0c0706'/>\n"
      "  </linearGradient>\n"
      // 火の粉の放射グラデ
      "  <radialGradient id='emberGrad' cx='50%%' cy='50%%' r='50%%'>\n"
      "    <stop offset='0%%'  stop-color='#ffd27a'/>\n"
      "    <stop offset='100%%' stop-color='rgba(255,100,0,0)'/>\n"
      "  </radialGradient>\n"
      // 煙のぼかし
      "  <filter id='smokeBlur'>\n"
      "    <feGaussianBlur stdDeviation='3'/>\n"
      "  </filter>\n"
      // 画面周辺のビネット
      "  <radialGradient id='vignetteGrad' cx='50%%' cy='50%%' r='75%%'>\n"
      "    <stop offset='60%%' stop-color='rgba(0,0,0,0)'/>\n"
      "    <stop offset='100%%' stop-color='rgba(0,0,0,0.55)'/>\n"
      "  </radialGradient>\n"
      // 遠景の山影に使うフェード
      "  <linearGradient id='ridgeFade' x1='0' y1='0' x2='0' y2='1'>\n"
      "    <stop offset='0%%'  stop-color='rgba(0,0,0,0.15)'/>\n"
      "    <stop offset='100%%' stop-color='rgba(0,0,0,0.55)'/>\n"
      "  </linearGradient>\n"
      // テキスト用の溶岩グラデーションとグロー
      "  <linearGradient id='textGrad' x1='0' y1='0' x2='1' y2='1'>\n"
      "    <stop offset='0%%'  stop-color='#fff2b3'/>\n"
      "    <stop offset='50%%' stop-color='#ffb24d'/>\n"
      "    <stop offset='100%%' stop-color='#ff6a00'/>\n"
      "  </linearGradient>\n"
      "  <filter id='textGlow'>\n"
      "    <feGaussianBlur in='SourceGraphic' stdDeviation='2'/>\n"
      "    <feMerge>\n"
      "      <feMergeNode/>\n"
      "      <feMergeNode in='SourceGraphic'/>\n"
      "    </feMerge>\n"
      "  </filter>\n"
      "</defs>\n"
      "<rect width='100%%' height='100%%' fill='url(#sky)'/>\n",
      W, H, W, H
    );

    // 遠景の稜線（パララックス感のある山影）
    printf("<g id='farRidge' opacity='0.65'>\n");
    printf("  <path d='M 0 %.1f C %.1f %.1f, %.1f %.1f, %.1f %.1f S %.1f %.1f, %.1f %.1f L %.1f %.1f L 0 %.1f Z' fill='url(#ridgeFade)'/>\n",
           H*0.58, W*0.18, H*0.48, W*0.36, H*0.62, W*0.52, H*0.56,
           W*0.72, H*0.66, W*0.95, H*0.60, W, H, 0.0, H);
    printf("</g>\n");

    // 低い煙の層（水平に漂う靄）
    printf("<rect x='0' y='%.1f' width='%.1f' height='%.1f' fill='rgba(255,180,120,0.03)'/>\n", H*0.50, W, H*0.10);

    // 下半分を溶岩グローでほんのり照らす
    printf("<rect x='0' y='%.1f' width='%.1f' height='%.1f' fill='url(#lavaGlow)' opacity='0.55'/>\n", H*0.55, W, H*0.50);

    // 火山（シルエット + クレーター + 噴煙）
    double vx = W * 0.85, vy = H * 0.88;   // 火山の基準位置（中央やや右）
    double vw = W * 0.34, vh = H * 0.50;   // 幅と高さ
    double peakX = vx, peakY = vy - vh;

    printf("<g id='volcano'>\n");

    // 山体（上端が小さな台形になるシルエット）
    double topY = peakY + 10.0;              // 三角頂点より少し下げて平らな天面
    double topW = vw * 0.12;                 // とても小さい台形幅
    double topL = peakX - topW * 0.5;        // 台形左端
    double topR = peakX + topW * 0.5;        // 台形右端
    printf("  <path d='M %.1f %.1f L %.1f %.1f L %.1f %.1f L %.1f %.1f Z'\n"
          "        fill='#1a1a1a' stroke='rgba(0,0,0,0.5)' stroke-width='2'/>\n",
          vx - vw*0.6, vy,                    // 左下
          vx + vw*0.6, vy,                    // 右下
          topR, topY,                         // 右上（台形上辺）
          topL, topY);                        // 左上（台形上辺）

    // 上へ立ち上る煙（関数化）
    draw_rising_smoke(stdout, peakX, topY, H, topW);

    printf("</g>\n");

    // 噴火口下の溶岩の溜まり（淡い脈動）
    double poolCX = vx, poolCY = vy + 10.0; // volcano base より少し下
    printf("<g id='lavaPool' opacity='0.75'>\n");
    printf("  <ellipse cx='%.1f' cy='%.1f' rx='%.1f' ry='%.1f' fill='url(#lavaPoolGrad)'>\n"
           "    <animate attributeName='opacity' values='0.55;0.85;0.55' dur='3.2s' repeatCount='indefinite'/>\n"
           "  </ellipse>\n",
           poolCX, poolCY, vw*0.45, 26.0);
    printf("</g>\n");

    // 火の粉（背景エフェクト）
    falling_embers(stdout, W, H);

        // 波レイヤー
    for (int i=0;i<layers;i++){
        char d[1<<16];
        int tiles = (int)ceil(W / L[i].lambda) + 2;
        wave_path(d, sizeof(d), W, H, -L[i].lambda, L[i].lambda, L[i].amp, L[i].y, tiles);

        // 横移動（-λ 分をループ）と縦ゆらぎ（wobbleA*sin）を合成
        double travel = L[i].lambda;
        double durX   = travel / L[i].speed;
        double wobA   = L[i].wobbleA;
        double wobT   = L[i].wobbleT;

        printf(
          "<g opacity='%.2f'>\n"
          "  <path d='%s' fill='%s' filter='url(#soft)'>\n"
          "    <animateTransform attributeName='transform' additive='sum' type='translate'\n"
          "      from='0 0' to='-%.2f 0' dur='%.2fs' repeatCount='indefinite'/>\n"
          "    <animateTransform attributeName='transform' additive='sum' type='translate'\n"
          "      values='0 0; 0 %.2f; 0 0; 0 -%.2f; 0 0' keyTimes='0;0.25;0.5;0.75;1'\n"
          "      dur='%.2fs' repeatCount='indefinite'/>\n"
          "  </path>\n"
          "</g>\n",
          L[i].opacity, d, L[i].fill, travel, durX,  wobA, wobA,  wobT
        );
    }

    // 名前
    double brandX = W * 0.04, brandY = H * 0.14;
    printf("<g id='branding' opacity='0.85'>\n");
    // 文字本体
    printf("<text x='%.1f' y='%.1f' font-family='Inter, Roboto, Segoe UI, Helvetica, Arial, sans-serif' "
           "font-weight='700' font-size='57' letter-spacing='1.2' "
           "fill='none' stroke='rgba(255,255,255,0.35)' stroke-width='1.2'>%s</text>\n",
           brandX, brandY, get_username());
    // 文字の光
    printf("<text x='%.1f' y='%.1f' font-family='Inter, Roboto, Segoe UI, Helvetica, Arial, sans-serif' "
           "font-weight='700' font-size='57' letter-spacing='1.2' "
           "fill='url(#textGrad)' filter='url(#textGlow)'>%s</text>\n",
           brandX, brandY, get_username());
    printf("</g>\n");

    // 背景矩形（淡い青）
    double boxX = brandX - 6.0;
    double boxY = brandY + 20.0;
    double boxWidth = 1000.0;
    double boxHeight = 140.0;
    printf("<rect x='%.1f' y='%.1f' width='%.1f' height='%.1f' rx='8' ry='8' "
           "fill='rgba(120,40,20,0.22)' stroke='rgba(255,120,60,0.40)' stroke-width='1.0'/>\n",
           boxX, boxY, boxWidth, boxHeight);

    // 情報パネル：使用言語
    printf("<g id='info' transform='translate(%.1f, %.1f)' opacity='0.85'>\n", brandX + 15.0 , boxY + 40.0);
    printf("  <text font-size='36' font-family='Inter, sans-serif' fill='url(#textGrad)'>%s</text>\n", get_first_title());
    printf("  <text y='42' x='15' font-size='30' fill='rgba(255,255,255,0.85)'>%s</text>\n", get_first_desc1());
    printf("  <text y='82' x='15' font-size='30' fill='rgba(255,255,255,0.85)'>%s</text>\n", get_first_desc2());
    printf("</g>\n");

    double barX = brandX + 500.0;   // 横位置（名前の少し右）
    double barY = brandY + 40.0;   // 開始位置（矩形の上端）
    double barHeight = 100.0;      // 棒の高さ
    printf("<rect x='%.1f' y='%.1f' width='4' height='%.1f' "
        "fill='rgba(255,120,60,0.45)' stroke='rgba(255,180,120,0.55)' "
        "rx='2' ry='2'/>\n",
        barX, barY, barHeight);

    // 情報パネル：使用言語
    printf("<g id='info' transform='translate(%.1f, %.1f)' opacity='0.85'>\n", barX + 25.0 , boxY + 40.0);
    printf("  <text font-size='36' font-family='Inter, sans-serif' fill='url(#textGrad)'>%s</text>\n", get_second_title());
    printf("  <text y='42' x='15' font-size='30' fill='rgba(255,255,255,0.85)'>%s</text>\n", get_second_desc1());
    printf("  <text y='82' x='15' font-size='30' fill='rgba(255,255,255,0.85)'>%s</text>\n", get_second_desc2());
    printf("</g>\n");

    // 周辺減光（ビネット）
    printf("<rect width='100%%' height='100%%' fill='url(#vignetteGrad)'/>\n");

    // SVG 終了
    printf("</svg>\n");
    return 0;
}