// Spirex Screensaver
// ---------------------
// Copyright (C) 2001, 2002, 2003 John Horigan
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//

#include "Color.h"
//#include "myWindows.h"


ColorHSB::ColorHSB(float h, float s, float b)
:	hue(h),
	sat(s),
	bright(b)
{
}

ColorHSB::ColorHSB()
:	hue(0.0F),
	sat(0.0F),
	bright(0.0F)
{
}

ColorRGB::ColorRGB(float r, float g, float b)
:	red(r),
	green(g),
	blue(b),
        alpha(1.0F)
{
}

ColorRGB::ColorRGB()
:	red(0.0F),
	green(0.0F),
	blue(0.0F),
        alpha(1.0F)
{
}

ColorRGB::ColorRGB(const ColorHSB& HSB)
{
	if (HSB.sat < 1.0F) {
		Set(HSB.bright, HSB.bright, HSB.bright);
		return;
	}

	int i = (int)(HSB.hue / 60.0F);
	float f = HSB.hue / 60.0F - i;
	float S = HSB.sat / 100.0F;
	float p = HSB.bright * (1.0F - S);
	float q = HSB.bright * (1.0F - S * f);
	float t = HSB.bright * (1.0F - S * (1.0F - f));
	switch (i) {
		case 0:
			Set(HSB.bright, t, p);
			break;
		case 1:
			Set(q, HSB.bright, p);
			break;
		case 2:
			Set(p, HSB.bright, t);
			break;
		case 3:
			Set(p, q, HSB.bright);
			break;
		case 4:
			Set(t, p, HSB.bright);
			break;
		default:	// was 5
			Set(HSB.bright, p, q);
			break;
	}
}


