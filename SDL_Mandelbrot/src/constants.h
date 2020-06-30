#ifndef COLOR_H
#define COLOR_H

constexpr int CHANNEL = 3;
constexpr int RGB888_SIZE = 4;
constexpr int COLOR_PALETTE_STRIDE = 3;
constexpr int THRESHOLD = 330;
constexpr int INTERVAL = 11;
constexpr uint8_t COLOR_BOUNDED[COLOR_PALETTE_STRIDE] = { 0, 0, 0 };
constexpr uint8_t COLOR_UNBOUNDED[COLOR_PALETTE_STRIDE * ((THRESHOLD + INTERVAL - 1) / INTERVAL)] = {
	0, 42, 147,
	15, 82, 175,
	31, 121, 201,
	50, 171, 235,
	64, 203, 231,

	68, 207, 201,
	72, 211, 176,
	74, 213, 156,
	77, 218, 132,
	80, 219, 110,

	84, 221, 92,
	87, 226, 60,
	93, 231, 22,
	110, 233, 18,
	126, 235, 17,

	148, 236, 19,
	156, 237, 19,
	172, 238, 19,
	185, 239, 19,
	203, 240, 19,

	220, 243, 18,
	235, 244, 18,
	242, 233, 19,
	244, 213, 20,
	245, 200, 21,

	246, 188, 22,
	248, 176, 22,
	250, 153, 23,
	252, 138, 23,
	253, 123, 24
	//20, 30, 240,
	//20, 40, 240,
	//20, 50, 240,
	//30, 70, 230,
	//50, 80, 200,
	//70, 120, 170,
	//110, 130, 120,
	//150, 130, 50,
	//200, 120, 40,
	//200, 70, 20
};
constexpr int RGB888_OFFSETR = 2;
constexpr int RGB888_OFFSETG = 1;
constexpr int RGB888_OFFSETB = 0;

inline void color(uint8_t* pixels, int pos, int it)
{
	pixels[pos + RGB888_OFFSETR] = COLOR_UNBOUNDED[(it / INTERVAL) * COLOR_PALETTE_STRIDE + 0];
	pixels[pos + RGB888_OFFSETG] = COLOR_UNBOUNDED[(it / INTERVAL) * COLOR_PALETTE_STRIDE + 1];
	pixels[pos + RGB888_OFFSETB] = COLOR_UNBOUNDED[(it / INTERVAL) * COLOR_PALETTE_STRIDE + 2];
}


#endif // !COLOR_H