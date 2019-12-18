#pragma once
namespace Graphics {
	// Color class represent 24bits color value, prevent inheritance
	class cColor final
	{
	public:
		/** public variable */
		float r, g, b;
		/** Constructors and destructor */
		cColor() { r = g = b = 0; };
		cColor(const cColor  &c) : r(c.r), g(c.g), b(c.b) {}
		explicit cColor(float i_r, float i_g, float i_b) : r(i_r), g(i_g), b(i_b) {}
		explicit cColor(float grey) : r(grey), g(grey), b(grey) {}
		~cColor() { r = g = b = 0; };

		/** public static functions*/
		static cColor Black() { return cColor(); }
		static cColor White() { return cColor(1.0, 1.0, 1.0); }

		/** Usage functions*/
		void SetColor(float i_r, float i_g, float i_b) { r = i_r; g = i_g; b = i_b; }

		/** operators*/
		cColor& operator = (const cColor& i_color) { r = i_color.r; g = i_color.g; b = i_color.b; return *this; }
		cColor  operator + (const cColor& i_color) const { return cColor(r + i_color.r, g + i_color.g, b + i_color.b); }
		cColor  operator - (const cColor& i_color) const { return cColor(r - i_color.r, g - i_color.g, b - i_color.b); }
		cColor  operator * (const cColor& i_color) const { return cColor(r*i_color.r, g*i_color.g, b*i_color.b); }
		cColor  operator / (const cColor& i_color) const { return cColor(r / i_color.r, g / i_color.g, b / i_color.b); }
		cColor  operator +  (const float & i_n) const { return cColor(r + i_n, g + i_n, b + i_n); }
		cColor  operator - (const float &i_n) const { return cColor(r - i_n, g - i_n, b - i_n); }
		cColor  operator * (const float &i_n) const { return cColor(r*i_n, g*i_n, b*i_n); }
		cColor  operator / (const float &i_n) const { return cColor(r / i_n, g / i_n, b / i_n); }
	private:

	};

}