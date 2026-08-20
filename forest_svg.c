#include <stdio.h>
#include "config.h"

static void stars(FILE *fp, double W, double H) {
    unsigned int seed = 418923u;
    for (int i = 0; i < 105; i++) {
        seed = seed * 1664525u + 1013904223u;
        double x = W * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double y = H * 0.48 * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double r = 0.35 + 0.85 * ((seed >> 8) / (double)0xFFFFFFu);
        double opacity = 0.25 + 0.50 * ((seed >> 8) / (double)0xFFFFFFu);
        fprintf(fp, "<circle cx='%.1f' cy='%.1f' r='%.2f' fill='#e7f5de' opacity='%.2f'><animate attributeName='opacity' values='%.2f;%.2f;%.2f' dur='4s' begin='%ds' repeatCount='indefinite'/></circle>\n", x, y, r, opacity, opacity, opacity * .35, opacity, i % 5);
    }
}

static void pines(FILE *fp, double W, int count, double y, double y_jitter, double scale, const char *const shades[], double opacity) {
    unsigned int seed = 71521u + (unsigned int)(scale * 100.0);
    /* 前景は小さな群れと空き間を作るため、意図的に不規則な配置にする。 */
    static const double foreground_x[] = { .005, .055, .115, .180, .245, .290,
                                            .355, .430, .495, .550, .620, .675,
                                            .735, .795, .850, .905, .955, .995 };
    for (int i = 0; i < count; i++) {
        seed = seed * 1103515245u + 12345u;
        double x;
        if (scale > 150.0) {
            x = W * foreground_x[i];
        } else {
            x = W * ((seed >> 8) / (double)0xFFFFFFu);
        }
        seed = seed * 1103515245u + 12345u;
        double h = scale * (0.75 + ((seed >> 8) / (double)0xFFFFFFu) * 0.55);
        seed = seed * 1103515245u + 12345u;
        double base_y = y + (((seed >> 8) / (double)0xFFFFFFu) - .5) * y_jitter;
        double w = h * .23;
        /* 重なった三角形で、枝葉が連続した針葉樹の輪郭を描く。 */
        fprintf(fp, "<g fill='%s' opacity='%.2f'>\n", shades[i % 4], opacity);
        fprintf(fp, "<path d='M %.1f %.1f L %.1f %.1f L %.1f %.1f Z'/>\n", x, base_y-h, x-w*.56, base_y-h*.43, x+w*.56, base_y-h*.43);
        fprintf(fp, "<path d='M %.1f %.1f L %.1f %.1f L %.1f %.1f Z'/>\n", x, base_y-h*.72, x-w*.83, base_y-h*.18, x+w*.83, base_y-h*.18);
        fprintf(fp, "<path d='M %.1f %.1f L %.1f %.1f L %.1f %.1f Z'/>\n", x, base_y-h*.48, x-w*1.13, base_y, x+w*1.13, base_y);
        fprintf(fp, "<rect x='%.1f' y='%.1f' width='%.1f' height='%.1f' rx='2'/>\n", x-w*.11, base_y-h*.18, w*.22, h*.30);
        fprintf(fp, "</g>\n");
    }
}

static void fireflies(FILE *fp, double W, double H) {
    unsigned int seed = 8831u;
    fprintf(fp, "<g filter='url(#fireflyGlow)'>\n");
    for (int i = 0; i < 45; i++) {
        seed = seed * 1664525u + 1013904223u;
        double x = W * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double y = H * (.58 + .34 * ((seed >> 8) / (double)0xFFFFFFu));
        seed = seed * 1664525u + 1013904223u;
        double duration = 3.4 + 5.2 * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double delay = -8.0 * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double drift_x = -12.0 + 24.0 * ((seed >> 8) / (double)0xFFFFFFu);
        seed = seed * 1664525u + 1013904223u;
        double drift_y = -3.0 - 14.0 * ((seed >> 8) / (double)0xFFFFFFu);
        fprintf(fp, "<circle cx='%.1f' cy='%.1f' r='1.8' fill='#d9ff72' opacity='0'><animate attributeName='opacity' values='0;.82;.12;.95;0' keyTimes='0;.18;.48;.72;1' dur='%.2fs' begin='%.2fs' repeatCount='indefinite'/><animateTransform attributeName='transform' type='translate' values='0 0; %.1f %.1f; %.1f %.1f; 0 0' dur='%.2fs' begin='%.2fs' repeatCount='indefinite'/></circle>\n", x, y, duration, delay, drift_x, drift_y, -drift_x*.35, drift_y*.30, duration * 1.31, delay * .73);
    }
    fprintf(fp, "</g>\n");
}

int main(void) {
    const double W = 1300, H = 490;
    const char *far_shades[] = { "#1d5035", "#214f37", "#19452f", "#24583d" };
    const char *middle_shades[] = { "#123b27", "#17462e", "#103520", "#1b4b31" };
    const char *near_shades[] = { "#082b1b", "#0b3421", "#10422a", "#062519" };
    printf("<svg xmlns='http://www.w3.org/2000/svg' width='%.0f' height='%.0f' viewBox='0 0 %.0f %.0f'>\n"
           "<defs>"
           "<linearGradient id='sky' x1='0' y1='0' x2='0' y2='1'><stop stop-color='#07150f'/><stop offset='.52' stop-color='#123525'/><stop offset='1' stop-color='#1d3822'/></linearGradient>"
           "<radialGradient id='moon' cx='50%%' cy='50%%' r='50%%'><stop stop-color='#fffde1'/><stop offset='.58' stop-color='#d9edb0'/><stop offset='1' stop-color='rgba(217,237,176,0)'/></radialGradient>"
           "<linearGradient id='textGrad' x1='0' y1='0' x2='1' y2='1'><stop stop-color='#edffc7'/><stop offset='.55' stop-color='#a9dc80'/><stop offset='1' stop-color='#65b78b'/></linearGradient>"
           "<filter id='glow'><feGaussianBlur stdDeviation='7'/></filter><filter id='fireflyGlow'><feGaussianBlur stdDeviation='.7'/></filter>"
           "<filter id='mist'><feGaussianBlur stdDeviation='13'/></filter>"
           "</defs><rect width='100%%' height='100%%' fill='url(#sky)'/>\n", W, H, W, H);

    stars(stdout, W, H);
    printf("<circle cx='1110' cy='94' r='84' fill='url(#moon)' opacity='.45' filter='url(#glow)'/><circle cx='1110' cy='94' r='30' fill='#f5f4d2'/>\n");
    printf("<path d='M0 310 C150 225 300 300 445 250 S725 270 870 214 S1130 278 1300 215 L1300 490 L0 490Z' fill='#173526' opacity='.75'/>\n");
    pines(stdout, W, 20, 405, 70, 95, far_shades, .55);
    printf("<g opacity='.38' filter='url(#mist)'><ellipse cx='260' cy='355' rx='300' ry='25' fill='#b5d6ac'><animateTransform attributeName='transform' type='translate' values='-50 0;60 4;-50 0' dur='19s' repeatCount='indefinite'/></ellipse><ellipse cx='930' cy='408' rx='390' ry='28' fill='#91b88d'><animateTransform attributeName='transform' type='translate' values='60 0;-45 -3;60 0' dur='23s' repeatCount='indefinite'/></ellipse></g>\n");
    pines(stdout, W, 15, 438, 55, 132, middle_shades, .78);
    pines(stdout, W, 13, 470, 30, 175, near_shades, .96);
    fireflies(stdout, W, H);

    double brandX = 52, brandY = 70, boxY = 90;
    double brandSize = brand_font_size(get_username());
    printf("<g opacity='.9'><text x='%.1f' y='%.1f' font-family='Inter,Roboto,Segoe UI,Helvetica,Arial,sans-serif' font-weight='700' font-size='%.1f' letter-spacing='1.2' fill='url(#textGrad)'>", brandX, brandY, brandSize);
    print_svg_escaped(stdout, get_username());
    printf("</text>\n");
    int firstHeight = info_column_height(get_first_title(), get_first_desc1(), get_first_desc2());
    int secondHeight = info_column_height(get_second_title(), get_second_desc1(), get_second_desc2());
    double boxHeight = (firstHeight > secondHeight ? firstHeight : secondHeight) + 32.0;
    printf("<rect x='46' y='%.1f' width='1000' height='%.1f' rx='8' fill='rgba(10,39,24,.44)' stroke='rgba(190,235,168,.30)'/>\n", boxY, boxHeight);
    printf("<g id='info-first'>\n");
    print_info_column(stdout, 67, boxY + 40, get_first_title(), get_first_desc1(), get_first_desc2(),
                      "url(#textGrad)", "#e9f3e3");
    printf("</g>\n<rect x='552' y='110' width='4' height='%.1f' rx='2' fill='rgba(183,232,160,.55)'/>\n", boxHeight - 40.0);
    printf("<g id='info-second'>\n");
    print_info_column(stdout, 581, boxY + 40, get_second_title(), get_second_desc1(), get_second_desc2(),
                      "url(#textGrad)", "#e9f3e3");
    printf("</g></g>\n");
    printf("<rect width='100%%' height='100%%' fill='none' stroke='rgba(177,220,145,.20)' stroke-width='10'/></svg>\n");
    return 0;
}
