#include <stdio.h>
#include <math.h>

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

static void star_field(FILE *fp, int count, double W, double H, double yMax) {
    // 上空に小さな星を配置（擬似乱数：LCG）
    unsigned int seed = 98765u;
    #define RND() (seed = 1664525u*seed + 1013904223u, (seed>>8) / (double)0xFFFFFFu)
    for (int i=0;i<count;i++){
        double x = W * RND();
        double y = yMax * RND();              // 0 ～ yMax に限定して上側だけに星
        double r = 0.4 + 1.1 * RND();
        // ごく薄い色温度の差
        double tint = RND();
        int R = (int)(255 * (0.85 + 0.15 * tint));
        int G = (int)(255 * (0.85 + 0.10 * (1.0 - tint)));
        int B = 255;
        double op = 0.35 + 0.45 * RND();
        int begin = (int)(RND() * 6.0);       // 0～5秒のランダム開始で瞬き
        fprintf(fp,
          "<circle cx='%.1f' cy='%.1f' r='%.2f' fill='rgb(%d,%d,%d)' opacity='%.2f'>"
          "<animate attributeName='opacity' values='%.2f;%.2f;%.2f' dur='4s' begin='%ds' repeatCount='indefinite'/></circle>\n",
          x, y, r, R, G, B, op, op*0.6, op*0.2, op*0.6, begin);
    }
    #undef RND
}

int main(void) {
    const double W = 1200, H = 420;

    Layer L[] = {
        //   λ     A     y       speed  wobA wobT  color      opac
        {  420,   18,  H*0.57,    22,   3.0, 15.0, "#0a395d", 0.60 }, // 奥（遅い・小さい）
        {  520,   26,  H*0.63,    32,   4.0, 18.0, "#0e5d8c", 0.62 },
        {  640,   34,  H*0.70,    45,   5.0, 22.0, "#147ab8", 0.60 },
        {  760,   46,  H*0.78,    60,   6.0, 26.0, "#1f95d6", 0.55 },
        {  920,   62,  H*0.86,    82,   7.0, 30.0, "#2cb6f0", 0.50 }  // 手前（速い・大きい）
    };
    const int layers = (int)(sizeof(L)/sizeof(L[0]));
    // const int tiles = 2; // 横方向タイル数（シームレス用）。2以上。  // Removed as per instructions

    printf(
      "<svg xmlns='http://www.w3.org/2000/svg' width='%.0f' height='%.0f' viewBox='0 0 %.0f %.0f'>\n"
      "<defs>\n"
      "  <linearGradient id='sky' x1='0' y1='0' x2='0' y2='1'>\n"
      "    <stop offset='0%%'  stop-color='#071523'/>\n"
      "    <stop offset='100%%' stop-color='#0b1f33'/>\n"
      "  </linearGradient>\n"
      "  <filter id='soft'>\n"
      "    <feGaussianBlur stdDeviation='0.5'/>\n"
      "  </filter>\n"
      "  <radialGradient id='moonGrad' cx='50%%' cy='50%%' r='50%%'>\n"
      "    <stop offset='0%%'  stop-color='#fffdf2'/>\n"
      "    <stop offset='60%%' stop-color='#f7f0d5'/>\n"
      "    <stop offset='100%%' stop-color='rgba(247,240,213,0)'/>\n"
      "  </radialGradient>\n"
      "  <filter id='glow'>\n"
      "    <feGaussianBlur stdDeviation='6'/>\n"
      "  </filter>\n"
      "  <linearGradient id='textGrad' x1='0' y1='0' x2='1' y2='1'>\n"
      "    <stop offset='0%%'  stop-color='#bfe9ff'/>\n"
      "    <stop offset='50%%' stop-color='#8fd3fe'/>\n"
      "    <stop offset='100%%' stop-color='#4facfe'/>\n"
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

    // 星（上部 0～40% の範囲に配置）
    printf("<g opacity='0.9'>\n");
    star_field(stdout, 140, W, H, H*0.40);
    printf("</g>\n");

    // 月（右上に配置）
    double moonX = W * 0.88, moonY = H * 0.18, moonR = 26.0;
    printf("<g>\n");
    // 外側の淡い光
    printf("<circle cx='%.1f' cy='%.1f' r='%.1f' fill='url(#moonGrad)' opacity='0.65' filter='url(#glow)'/>\n",
           moonX, moonY, moonR * 2.6);
    // 本体
    printf("<circle cx='%.1f' cy='%.1f' r='%.1f' fill='#fffbe8' stroke='rgba(255,255,255,0.5)' stroke-width='0.8'/>\n",
           moonX, moonY, moonR);
    printf("</g>\n");

    // 名前
    double brandX = W * 0.04, brandY = H * 0.19;
    printf("<g id='branding' opacity='0.85'>\n");
    // 文字本体
    printf("<text x='%.1f' y='%.1f' font-family='Inter, Roboto, Segoe UI, Helvetica, Arial, sans-serif' "
           "font-weight='700' font-size='45' letter-spacing='1.2' "
           "fill='none' stroke='rgba(255,255,255,0.35)' stroke-width='1.2'>Aotumuri</text>\n",
           brandX, brandY);
    // 文字の光
    printf("<text x='%.1f' y='%.1f' font-family='Inter, Roboto, Segoe UI, Helvetica, Arial, sans-serif' "
           "font-weight='700' font-size='45' letter-spacing='1.2' "
           "fill='url(#textGrad)' filter='url(#textGlow)'>Aotumuri</text>\n",
           brandX, brandY);
    printf("</g>\n");

    // 情報パネル
    printf("<!-- info panel placeholder -->\n");
    printf("<g id='info' transform='translate(%.1f, %.1f)'>\n", W*0.62, H*0.18);
    printf("  <!-- future: commit stats, streaks, etc. populated via pre-rendered text -->\n");
    printf("</g>\n");

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

    printf("</svg>\n");
    return 0;
}