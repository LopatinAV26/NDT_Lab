#include "nomogramWindow.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <limits>
#include <optional>
#include "implot.h"
#include "applicationData.hpp"
#include "utilities.hpp"

namespace
{
	// Холст схемы: мировые координаты в мм (ось y вниз), экранные — в пикселях
	struct SchemeCanvas
	{
		ImDrawList *dl = nullptr;
		ImVec2 origin{};   // экранная точка мировой (0, 0)
		float scale = 1.f; // пикселей на мм
		ImU32 line = 0;
		ImU32 wall = 0;
		ImU32 film = IM_COL32(90, 170, 255, 255);
		ImU32 ray = IM_COL32(255, 200, 60, 170);
		ImU32 source = IM_COL32(255, 80, 60, 255);
		ImU32 beam = IM_COL32(255, 200, 60, 40);

		ImVec2 P(float x, float y) const { return {origin.x + x * scale, origin.y + y * scale}; }
		float Px(float mm) const { return mm * scale; }
		float Mm(float px) const { return px / scale; }
	};

	constexpr float filmPx = 4.f;
	constexpr float pi = 3.14159265f;

	// Масштаб, при котором мировой прямоугольник входит в экранный
	float FitScale(ImVec2 wMin, ImVec2 wMax, ImVec2 sMin, ImVec2 sMax)
	{
		const float w = std::max(wMax.x - wMin.x, 1e-3f);
		const float h = std::max(wMax.y - wMin.y, 1e-3f);
		return std::max(std::min((sMax.x - sMin.x) / w, (sMax.y - sMin.y) / h), 1e-3f);
	}

	// Ставит центр мирового прямоугольника в центр экранного
	void CenterView(SchemeCanvas &c, ImVec2 wMin, ImVec2 wMax, ImVec2 sMin, ImVec2 sMax)
	{
		c.origin = {(sMin.x + sMax.x) / 2.f - (wMin.x + wMax.x) / 2.f * c.scale,
					(sMin.y + sMax.y) / 2.f - (wMin.y + wMax.y) / 2.f * c.scale};
	}

	// openEnds — без левой и правой сторон: там общая линия обрыва (BreakLine)
	void HatchRect(const SchemeCanvas &c, float x0, float y0, float x1, float y1, bool openEnds = false)
	{
		const ImVec2 a = c.P(x0, y0);
		const ImVec2 b = c.P(x1, y1);
		const ImVec2 mn{std::min(a.x, b.x), std::min(a.y, b.y)};
		const ImVec2 mx{std::max(a.x, b.x), std::max(a.y, b.y)};
		const float h = mx.y - mn.y;

		c.dl->AddRectFilled(mn, mx, c.wall);
		c.dl->PushClipRect(mn, mx, true);
		for (float x = mn.x - h; x < mx.x; x += 6.f)
			c.dl->AddLine({x, mx.y}, {x + h, mn.y}, c.line);
		c.dl->PopClipRect();
		if (openEnds)
		{
			c.dl->AddLine(mn, {mx.x, mn.y}, c.line, 1.5f);
			c.dl->AddLine({mn.x, mx.y}, mx, c.line, 1.5f);
		}
		else
			c.dl->AddRect(mn, mx, c.line, 0.f, 0, 1.5f);
	}

	// Линия обрыва: прямая по вертикали x от y0 до y1
	void BreakLine(const SchemeCanvas &c, float x, float y0, float y1)
	{
		c.dl->AddLine(c.P(x, y0), c.P(x, y1), c.line, 1.f);
	}

	void HatchRing(const SchemeCanvas &c, float cx, float cy, float rIn, float rOut)
	{
		const ImVec2 ctr = c.P(cx, cy);
		const float ri = c.Px(rIn);
		const float ro = c.Px(rOut);

		// в ImGui 1.93 обводка окружности по умолчанию внутрь: полоса ro - ri от ro к центру
		c.dl->AddCircle(ctr, ro, c.wall, 0, ro - ri);

		// штриховка под 45°, как в HatchRect: прямые со сдвигом d от центра по нормали,
		// внутри кольца — хорда наружной окружности без хорды внутренней
		const float k = 0.70710678f;
		const ImVec2 u{k, -k};		// направление штриха
		const ImVec2 n{k, k};		// нормаль
		const float step = 6.f * k; // тот же шаг, что у HatchRect (6 px по горизонтали)
		auto segment = [&](float d, float s0, float s1)
		{
			c.dl->AddLine({ctr.x + n.x * d + u.x * s0, ctr.y + n.y * d + u.y * s0},
						  {ctr.x + n.x * d + u.x * s1, ctr.y + n.y * d + u.y * s1}, c.line);
		};
		for (float d = -ro + step / 2.f; d < ro; d += step)
		{
			const float ho = std::sqrt(ro * ro - d * d);
			if (std::fabs(d) < ri)
			{
				const float hi = std::sqrt(ri * ri - d * d);
				segment(d, -ho, -hi);
				segment(d, hi, ho);
			}
			else
				segment(d, -ho, ho);
		}
		c.dl->AddCircle(ctr, ri, c.line, 0, 1.5f);
		c.dl->AddCircle(ctr, ro, c.line, 0, 1.5f);
	}

	// Шов: основание треугольника на наружной поверхности, вершина — на внутренней,
	// наружу от основания — выпуклость высотой bulge (мм)
	void Weld(const SchemeCanvas &c, float x, float yOuter, float yInner, float bulge)
	{
		const float half = std::max(std::fabs(yInner - yOuter) * 0.7f, c.Mm(3.f));
		const float dir = (yOuter > yInner) ? 1.f : -1.f; // куда наружу

		constexpr int arcPoints = 16;
		ImVec2 poly[arcPoints + 2];
		poly[0] = c.P(x, yInner);
		for (int i = 0; i <= arcPoints; ++i)
		{
			// дуга окружности по хорде 2 * half со стрелкой bulge
			const float s = -1.f + 2.f * static_cast<float>(i) / arcPoints;
			const float rho = (half * half + bulge * bulge) / (2.f * std::max(bulge, 1e-3f));
			const float px = s * half;
			const float py = std::sqrt(std::max(rho * rho - px * px, 0.f)) - (rho - bulge);
			poly[i + 1] = c.P(x + px, yOuter + dir * py);
		}
		c.dl->AddConvexPolyFilled(poly, arcPoints + 2, c.line);
	}

	// Обратный валик (корень шва): сегмент дуги шириной 2 * half и высотой bulge (мм)
	// на поверхности y; dir = +1 — выпуклость вниз, -1 — вверх
	void RootBead(const SchemeCanvas &c, float x, float y, float half, float bulge, float dir)
	{
		half = std::max(half, c.Mm(1.f));
		constexpr int arcPoints = 12;
		ImVec2 poly[arcPoints + 1];
		const float rho = (half * half + bulge * bulge) / (2.f * std::max(bulge, 1e-3f));
		for (int i = 0; i <= arcPoints; ++i)
		{
			const float px = (-1.f + 2.f * static_cast<float>(i) / arcPoints) * half;
			const float py = std::sqrt(std::max(rho * rho - px * px, 0.f)) - (rho - bulge);
			poly[i] = c.P(x + px, y + dir * py);
		}
		c.dl->AddConvexPolyFilled(poly, arcPoints + 1, c.line);
	}

	// Линия стыка между швами противоположных стенок
	void JointLine(const SchemeCanvas &c, float x, float y0, float y1)
	{
		c.dl->AddLine(c.P(x, y0), c.P(x, y1), c.line, 2.f);
	}

	// Изображение шва на плоском детекторе (ниже y): тёмная полоса шириной 2 * half
	void WeldImage(const SchemeCanvas &c, float x, float y, float half)
	{
		const ImVec2 a = c.P(x - half, y);
		const ImVec2 b = c.P(x + half, y);
		c.dl->AddRectFilled({a.x, a.y + 1.f}, {std::max(b.x, a.x + 2.f), a.y + filmPx - 1.f}, c.line);
	}

	// lg экспозиции на кривой при толщине x: кривые — ломаные на логарифмической шкале,
	// за концами продолжаются крайние отрезки. Толщины кривой делятся на xScale
	float CurveLogExposure(const Curve &curve, float x, float xScale)
	{
		const auto &xs = curve.xData;
		const auto &ys = curve.yData;
		const size_t n = std::min(xs.size(), ys.size());
		if (n == 0)
			return 0.f;
		if (n == 1)
			return std::log10(ys[0]);
		size_t i = 0;
		while (i + 2 < n && x > xs[i + 1] / xScale)
			++i;
		const float x0 = xs[i] / xScale;
		const float x1 = xs[i + 1] / xScale;
		const float l0 = std::log10(ys[i]);
		const float l1 = std::log10(ys[i + 1]);
		if (std::fabs(x1 - x0) < 1e-6f)
			return l0;
		return l0 + (x - x0) / (x1 - x0) * (l1 - l0);
	}

	// Соседние по напряжению кривые вокруг u и доля u между ними. Только внутри диапазона
	// напряжений кривых и при хотя бы двух разных напряжениях, иначе nullopt
	struct VoltageNeighbors
	{
		const Curve *a;
		const Curve *b;
		float share; // 0 — на кривой a, 1 — на кривой b
	};
	std::optional<VoltageNeighbors> NeighborCurves(const std::vector<Curve> &curves, float u)
	{
		std::vector<const Curve *> sorted;
		for (const Curve &curve : curves)
			sorted.push_back(&curve);
		std::sort(sorted.begin(), sorted.end(), [](const Curve *a, const Curve *b) { return a->voltage < b->voltage; });
		// одинаковые напряжения (разные плёнки) для интерполяции не годятся — берём первую
		sorted.erase(std::unique(sorted.begin(), sorted.end(), [](const Curve *a, const Curve *b)
								 { return std::fabs(a->voltage - b->voltage) < 0.5f; }),
					 sorted.end());
		if (sorted.size() < 2 || u < sorted.front()->voltage || u > sorted.back()->voltage)
			return std::nullopt;

		size_t k = 0;
		while (k + 2 < sorted.size() && u > sorted[k + 1]->voltage)
			++k;
		const Curve *a = sorted[k];
		const Curve *b = sorted[k + 1];
		return VoltageNeighbors{a, b, (u - a->voltage) / (b->voltage - a->voltage)};
	}

	// Промежуточная кривая по точкам: каждая её точка — между соответствующими точками соседних
	// кривых по доле напряжения (толщина линейно, экспозиция — по логарифму). Так пробиваемая
	// толщина растёт с напряжением линейно, как у самих номограмм. Если число точек у кривых
	// разное — по первой и последней. Толщины делятся на xScale
	Curve InterpolatedCurve(const VoltageNeighbors &n, float xScale)
	{
		const Curve &a = *n.a;
		const Curve &b = *n.b;
		Curve result;
		result.voltage = a.voltage + n.share * (b.voltage - a.voltage);
		auto add = [&](size_t ia, size_t ib)
		{
			result.xData.push_back((a.xData[ia] + n.share * (b.xData[ib] - a.xData[ia])) / xScale);
			const float la = std::log10(a.yData[ia]);
			const float lb = std::log10(b.yData[ib]);
			result.yData.push_back(std::pow(10.f, la + n.share * (lb - la)));
		};
		if (a.xData.size() == b.xData.size() && a.xData.size() >= 2)
			for (size_t i = 0; i < a.xData.size(); ++i)
				add(i, i);
		else
		{
			add(0, 0);
			add(a.xData.size() - 1, b.xData.size() - 1);
		}
		return result;
	}

	// Вид на детектор при просвечивании на эллипс в прямоугольнике rMin-rMax — как на снимке:
	// плёнка тёмная, труба светлее там, где луч проходит больший путь в металле (у краёв),
	// шов — светлая полоса по проекции кольца шва из источника.
	// По горизонтали — поперёк трубы (длина детектора filmLen), по вертикали — вдоль трубы
	// (ширина filmWidth). f — от источника до трубы, s — смещение источника вдоль трубы, мм
	void EllipseDetectorView(const SchemeCanvas &c, ImVec2 rMin, ImVec2 rMax, float D, float t, float f,
							 float bulge, float s, float filmLen, float filmWidth)
	{
		const float rounding = ImGui::GetStyle().FrameRounding;
		c.dl->AddRectFilled(rMin, rMax, ImGui::GetColorU32(ImGuiCol_WindowBg), rounding);
		c.dl->AddRect(rMin, rMax, ImGui::GetColorU32(ImGuiCol_Border), rounding);

		const float pad = 6.f;
		const float sc = std::min((rMax.x - rMin.x - 2.f * pad) / filmLen, (rMax.y - rMin.y - 2.f * pad) / filmWidth);
		const ImVec2 mid{(rMin.x + rMax.x) / 2.f, (rMin.y + rMax.y) / 2.f};
		auto Q = [&](float across, float along) { return ImVec2{mid.x + across * sc, mid.y + along * sc}; };

		const ImVec2 f0 = Q(-filmLen / 2.f, -filmWidth / 2.f);
		const ImVec2 f1 = Q(filmLen / 2.f, filmWidth / 2.f);
		c.dl->AddRectFilled(f0, f1, IM_COL32(12, 12, 14, 255)); // засвеченная плёнка
		c.dl->PushClipRect(f0, f1, true); // на детекторе — только то, что на него попадает

		if (f > 0.f)
		{
			// источник на высоте 0, ось трубы на f + R, детектор на f + D + bulge
			const float R = D / 2.f;
			const float zc = f + R;
			const float zf = f + D + bulge;
			const float r = std::max(R - t, 0.f);

			// края изображения трубы — следы касательных плоскостей из источника
			const float wall = zf * R / std::sqrt(zc * zc - R * R);

			// путь луча через обе стенки на расстоянии y от оси (луч почти вертикален)
			auto path = [&](float y)
			{
				y = std::fabs(y);
				if (y >= R)
					return 0.f;
				const float outer = 2.f * std::sqrt(R * R - y * y);
				return y < r ? outer - 2.f * std::sqrt(r * r - y * y) : outer;
			};
			// яркость на снимке: больше металла — светлее; через две стенки в центре — серый
			const float k = -std::log(0.55f) / std::max(2.f * t, 0.1f);
			auto shade = [&](float across)
			{
				const float y = across * R / wall; // назад на трубу
				if (std::fabs(y) >= R)
					return IM_COL32(12, 12, 14, 255);
				const auto g = static_cast<int>(40.f + 205.f * (1.f - std::exp(-k * path(y))));
				return IM_COL32(g, g, g, 255);
			};

			// тело трубы — вертикальные полосы с горизонтальным градиентом
			const float x0 = Q(-wall, 0.f).x;
			const float x1 = Q(wall, 0.f).x;
			const int columns = std::max(8, static_cast<int>((x1 - x0) / 2.f));
			for (int i = 0; i < columns; ++i)
			{
				const float a0 = -wall + 2.f * wall * static_cast<float>(i) / columns;
				const float a1 = -wall + 2.f * wall * static_cast<float>(i + 1) / columns;
				const ImU32 cl = shade(a0);
				const ImU32 cr = shade(a1);
				c.dl->AddRectFilledMultiColor({Q(a0, 0.f).x, f0.y}, {Q(a1, 0.f).x, f1.y}, cl, cr, cr, cl);
			}

			// шов — светлая полоса: кольцо валика (радиус R + bulge) шириной 2·hw вдоль трубы,
			// каждая точка проецируется из источника. Ширина полосы откладывается вдоль трубы,
			// поэтому по бокам шов выступает за трубу только на высоту валика
			const float Rw = R + bulge;
			constexpr int n = 96;
			const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
			auto band = [&](float hw, ImU32 col)
			{
				c.dl->PrimReserve(6 * n, 2 * (n + 1));
				const auto base = static_cast<ImDrawIdx>(c.dl->_VtxCurrentIdx);
				for (int i = 0; i <= n; ++i)
				{
					const float phi = 2.f * pi * static_cast<float>(i) / n;
					const float lambda = zf / (zc + Rw * std::cos(phi)); // увеличение для этой точки
					const float across = Rw * std::sin(phi) * lambda;
					c.dl->PrimWriteVtx(Q(across, s + (-hw - s) * lambda), uv, col);
					c.dl->PrimWriteVtx(Q(across, s + (hw - s) * lambda), uv, col);
				}
				for (int i = 0; i < n; ++i)
				{
					const auto a0 = static_cast<ImDrawIdx>(base + 2 * i);
					const ImDrawIdx idx[6] = {a0, static_cast<ImDrawIdx>(a0 + 1), static_cast<ImDrawIdx>(a0 + 3),
											  a0, static_cast<ImDrawIdx>(a0 + 3), static_cast<ImDrawIdx>(a0 + 2)};
					for (ImDrawIdx id : idx)
						c.dl->PrimWriteIdx(id);
				}
			};
			const float hw = 0.7f * t; // полуширина шва вдоль трубы, как на разрезе
			band(1.6f * hw, IM_COL32(255, 255, 255, 50)); // размытое свечение
			band(hw, IM_COL32(240, 240, 240, 140));		  // плотный валик
		}
		c.dl->PopClipRect();
		c.dl->AddRect(f0, f1, c.film); // рамка детектора
	}

	// Плоский детектор: dir = +1 — рисуется ниже y, -1 — выше
	void FlatFilm(const SchemeCanvas &c, float x0, float x1, float y, float dir)
	{
		const ImVec2 a = c.P(x0, y);
		const ImVec2 b{c.P(x1, y).x, a.y + dir * filmPx};
		const ImVec2 mn{std::min(a.x, b.x), std::min(a.y, b.y)};
		const ImVec2 mx{std::max(a.x, b.x), std::max(a.y, b.y)};
		c.dl->AddRectFilled(mn, mx, c.film);
		c.dl->AddRect(mn, mx, c.line);
	}

	// Изогнутый детектор по наружной поверхности; углы экранные (0 — вправо, pi/2 — вниз)
	void ArcFilm(const SchemeCanvas &c, float cx, float cy, float r, float a0, float a1)
	{
		c.dl->PathArcTo(c.P(cx, cy), c.Px(r) + filmPx / 2.f + 1.f, a0, a1);
		c.dl->PathStroke(c.film, 0, filmPx);
	}

	// Число экспозиций для фронтальной схемы по ГОСТ 7512-82 (черт. 5г, д):
	// N = 180° / (arcsin(pm) + arcsin(pm / (2n + 1))), m = d/D, n = f/D, p = √(1 − 0,2·(2,6 − 1/m)²).
	// D — наружный диаметр, t — стенка, f — от источника до ближней наружной поверхности трубы, мм
	int FrontalExposures(float D, float t, float f)
	{
		const float m = std::clamp((D - 2.f * t) / D, 0.01f, 1.f);
		const float n = std::max(f, 0.f) / D;
		const float q = 2.6f - 1.f / m;
		const float pm = std::sqrt(std::max(1.f - 0.2f * q * q, 0.f)) * m;
		const float half = std::asin(std::min(pm, 1.f)) + std::asin(pm / (2.f * n + 1.f)); // рад
		// номограмма отраслевого РД начинается с N = 3 — меньше не назначаем
		return std::max(3, static_cast<int>(std::ceil(pi / std::max(half, 1e-3f) - 1e-3f)));
	}

	void Source(const SchemeCanvas &c, float x, float y)
	{
		const ImVec2 p = c.P(x, y);
		c.dl->AddCircleFilled(p, 4.f, c.source);
		c.dl->AddText({p.x + 6.f, p.y - ImGui::GetTextLineHeight()}, c.line, "S");
	}

	// Пучок аппарата: сектор с вершиной в (x, y), ось — экранный угол axis (pi/2 — вниз),
	// раствор angleDeg. До fullPx (объект с детектором) заливка ровная, дальше за fadePx
	// плавно исчезает
	void Beam(const SchemeCanvas &c, float x, float y, float axis, float angleDeg, float fullPx, float fadePx)
	{
		const ImVec2 p = c.P(x, y);
		const float span = std::clamp(angleDeg, 1.f, 360.f) * pi / 180.f;
		const float a0 = axis - span / 2.f;
		const float outPx = fullPx + fadePx;
		const ImU32 clear = c.beam & ~IM_COL32_A_MASK;
		const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

		// широкий пучок (конус, полусфера) гаснет и к краям сектора, узкий — с резкими границами
		const bool wide = span > pi / 2.f && span < 2.f * pi - 1e-3f;
		const float beamAlpha = static_cast<float>((c.beam >> IM_COL32_A_SHIFT) & 0xFF);
		auto edgeColor = [&](float a)
		{
			if (!wide)
				return c.beam;
			const float t = std::clamp((std::fabs(a - axis) / (span / 2.f) - 0.4f) / 0.6f, 0.f, 1.f);
			const float w = 1.f - t * t * (3.f - 2.f * t); // smoothstep
			return clear | (static_cast<ImU32>(beamAlpha * w) << IM_COL32_A_SHIFT);
		};

		// веер из треугольников с цветом в вершинах: ровная часть + кольцо градиента
		const int n = std::max(4, static_cast<int>(span / (2.f * pi / 180.f))); // шаг около 2°
		c.dl->PrimReserve(9 * n, 1 + 2 * (n + 1));
		const auto base = static_cast<ImDrawIdx>(c.dl->_VtxCurrentIdx);
		c.dl->PrimWriteVtx(p, uv, c.beam);
		for (int k = 0; k <= n; ++k)
		{
			const float a = a0 + span * static_cast<float>(k) / static_cast<float>(n);
			const ImVec2 d{std::cos(a), std::sin(a)};
			c.dl->PrimWriteVtx({p.x + d.x * fullPx, p.y + d.y * fullPx}, uv, edgeColor(a));
			c.dl->PrimWriteVtx({p.x + d.x * outPx, p.y + d.y * outPx}, uv, clear);
		}
		for (int k = 0; k < n; ++k)
		{
			const auto i0 = static_cast<ImDrawIdx>(base + 1 + 2 * k); // ровная граница, луч k
			const auto j0 = static_cast<ImDrawIdx>(i0 + 2);			  // то же, луч k + 1
			const ImDrawIdx tri[9] = {base, i0, j0,
									  i0, static_cast<ImDrawIdx>(i0 + 1), static_cast<ImDrawIdx>(j0 + 1),
									  i0, static_cast<ImDrawIdx>(j0 + 1), j0};
			for (ImDrawIdx idx : tri)
				c.dl->PrimWriteIdx(idx);
		}
	}

	void Ray(const SchemeCanvas &c, float x0, float y0, float x1, float y1)
	{
		c.dl->AddLine(c.P(x0, y0), c.P(x1, y1), c.ray, 1.5f);
	}

	void AxisLine(const SchemeCanvas &c, float x0, float y0, float x1, float y1)
	{
		const ImVec2 a = c.P(x0, y0);
		const ImVec2 b = c.P(x1, y1);
		const float len = std::hypot(b.x - a.x, b.y - a.y);
		if (len < 1.f)
			return;
		const ImVec2 d{(b.x - a.x) / len, (b.y - a.y) / len};
		// штрихпунктир: штрих 12, пробел 3, точка 2, пробел 3
		for (float s = 0.f; s < len; s += 20.f)
		{
			const float e = std::min(s + 12.f, len);
			c.dl->AddLine({a.x + d.x * s, a.y + d.y * s}, {a.x + d.x * e, a.y + d.y * e}, c.line);
			if (s + 17.f < len)
				c.dl->AddLine({a.x + d.x * (s + 15.f), a.y + d.y * (s + 15.f)},
							  {a.x + d.x * (s + 17.f), a.y + d.y * (s + 17.f)}, c.line);
		}
	}

	// Вертикальный размер с выносными линиями от xFrom0 (на y0) и xFrom1 (на y1).
	// textLeft — подпись слева от размерной линии; breakAt — высота знака разрыва в мм
	// (расстояние показано не в масштабе), NaN — без разрыва
	constexpr float noBreak = std::numeric_limits<float>::quiet_NaN();
	void DimV(const SchemeCanvas &c, float x, float y0, float y1, float xFrom0, float xFrom1,
			  const char *text, bool textLeft, bool arrowsOutside = false, float breakAt = noBreak)
	{
		const ImVec2 a = c.P(x, y0);
		const ImVec2 b = c.P(x, y1);
		const float ext = (x > xFrom0 ? 4.f : -4.f);

		c.dl->AddLine(c.P(xFrom0, y0), {a.x + ext, a.y}, c.line, 0.5f);
		c.dl->AddLine(c.P(xFrom1, y1), {b.x + ext, b.y}, c.line, 0.5f);

		const float s = 6.f;
		const float dir = (b.y > a.y) ? 1.f : -1.f;
		if (arrowsOutside)
		{
			// стрелки снаружи остриями друг к другу, размерная линия выходит за выносные
			const float tail = s + 6.f;
			c.dl->AddLine({a.x, a.y - dir * tail}, {b.x, b.y + dir * tail}, c.line);
			c.dl->AddTriangleFilled(a, {a.x + s / 3.f, a.y - dir * s}, {a.x - s / 3.f, a.y - dir * s}, c.line);
			c.dl->AddTriangleFilled(b, {b.x - s / 3.f, b.y + dir * s}, {b.x + s / 3.f, b.y + dir * s}, c.line);
		}
		else
		{
			c.dl->AddLine(a, b, c.line);
			c.dl->AddTriangleFilled(a, {a.x - s / 3.f, a.y + dir * s}, {a.x + s / 3.f, a.y + dir * s}, c.line);
			c.dl->AddTriangleFilled(b, {b.x + s / 3.f, b.y - dir * s}, {b.x - s / 3.f, b.y - dir * s}, c.line);
		}

		const float midY = (a.y + b.y) / 2.f;
		if (!std::isnan(breakAt))
		{
			const float by = c.P(x, breakAt).y;
			c.dl->AddLine({a.x - 5.f, by - 1.f}, {a.x + 5.f, by - 5.f}, c.line, 1.5f);
			c.dl->AddLine({a.x - 5.f, by + 5.f}, {a.x + 5.f, by + 1.f}, c.line, 1.5f);
		}

		const ImVec2 ts = ImGui::CalcTextSize(text);
		const float tx = textLeft ? a.x - ts.x - 6.f : a.x + 6.f;
		c.dl->AddText({tx, midY - ts.y / 2.f}, c.line, text);
	}

	// Горизонтальный размер на высоте y с выносными линиями от yFrom0 (на x0) и yFrom1 (на x1),
	// подпись над размерной линией
	void DimH(const SchemeCanvas &c, float y, float x0, float x1, float yFrom0, float yFrom1, const char *text)
	{
		const ImVec2 a = c.P(x0, y);
		const ImVec2 b = c.P(x1, y);
		const float ext = (y < yFrom0 ? -4.f : 4.f);

		c.dl->AddLine(c.P(x0, yFrom0), {a.x, a.y + ext}, c.line, 0.5f);
		c.dl->AddLine(c.P(x1, yFrom1), {b.x, b.y + ext}, c.line, 0.5f);
		c.dl->AddLine(a, b, c.line);

		const float s = 6.f;
		const float dir = (b.x > a.x) ? 1.f : -1.f;
		c.dl->AddTriangleFilled(a, {a.x + dir * s, a.y - s / 3.f}, {a.x + dir * s, a.y + s / 3.f}, c.line);
		c.dl->AddTriangleFilled(b, {b.x - dir * s, b.y + s / 3.f}, {b.x - dir * s, b.y - s / 3.f}, c.line);

		const ImVec2 ts = ImGui::CalcTextSize(text);
		c.dl->AddText({(a.x + b.x) / 2.f - ts.x / 2.f, a.y - ts.y - 2.f}, c.line, text);
	}
}

NomogramWindow::NomogramWindow(ApplicationData &coreAppData, ResourceManager &resManager)
{
	devices = resManager.LoadDevices(coreAppData.pathToDevices);
	calculatedDevices = devices;
}

void NomogramWindow::Show()
{
	if (devices.empty() || calculatedDevices.empty())
	{
		ImGui::Text("No devices loaded.");
		return;
	}

	// слева параметры и номограмма, справа схема; всё по высоте окна, без прокрутки
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float controlsWidth = std::max(380.f, ImGui::GetContentRegionAvail().x * 0.45f);
	ImGui::BeginChild("##controls", ImVec2(controlsWidth, 0.f));
	ImGui::PushItemWidth(-FLT_MIN); // поля и график — на всю ширину левой части

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("(?)").x -
							ImGui::CalcTextSize("Паспорт").x - 2.f * spacing);
	if (ImGui::BeginCombo("##Аппарат#", calculatedDevices[deviceIndex].name.c_str()))
	{
		for (int i = 0; i < std::ssize(calculatedDevices); ++i)
		{
			const bool isSelected = (deviceIndex == i);
			if (ImGui::Selectable(calculatedDevices[i].name.c_str(), isSelected))
				deviceIndex = i;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}

		if (deviceCurrent < calculatedDevices[deviceIndex].currentMinimum ||
			deviceCurrent > calculatedDevices[deviceIndex].currentMaximum)
			deviceCurrent = calculatedDevices[deviceIndex].currentMinimum;

		ImGui::EndCombo();
	}
	ImGui::SameLine();

	NDT::HelpMarker(calculatedDevices[deviceIndex].information.c_str());
	ImGui::SameLine();
	ImGui::TextLinkOpenURL("Паспорт", "resources/devices_manual/Arina.pdf");

	static constexpr const char *schemeNames[] = {"Панорамная", "Фронтальная", "На эллипс", "Пластина"};
	static_assert(std::size(schemeNames) == static_cast<size_t>(Scheme::Count));

	// допустимый наружный диаметр для схемы и значение при её выборе, мм
	struct DiameterRange
	{
		float min;
		float max;
		float byDefault;
	};
	static constexpr DiameterRange diameterRanges[] = {
		{159.f, 2000.f, 1067.f}, // панорамная
		{159.f, 2000.f, 1067.f}, // фронтальная
		{10.f, 108.f, 57.f},	 // на эллипс
		{10.f, 2000.f, 159.f},	 // пластина — диаметр не используется
	};
	static_assert(std::size(diameterRanges) == static_cast<size_t>(Scheme::Count));

	for (int i = 0; i < static_cast<int>(Scheme::Count); ++i)
	{
		if (i > 0)
			ImGui::SameLine();
		if (ImGui::RadioButton(schemeNames[i], scheme == static_cast<Scheme>(i)) && scheme != static_cast<Scheme>(i))
		{
			scheme = static_cast<Scheme>(i);
			pipeDiameter = diameterRanges[i].byDefault;
		}
	}

	const bool isPipe = scheme != Scheme::Plate;
	// через две стенки просвечивается удвоенная толщина
	const float wallFactor = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? 2.f : 1.f;

	const DiameterRange &range = diameterRanges[static_cast<int>(scheme)];
	pipeDiameter = std::clamp(pipeDiameter, range.min, range.max);

	ImGui::BeginDisabled(!isPipe);
	ImGui::DragFloat("##Диаметр#", &pipeDiameter, 1.f, range.min, range.max, "Наружный диаметр: %.0f мм",
					 ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();

	// у панорамной фокусное расстояние — радиус трубы; у остальных труб источник не ближе
	// поверхности; детектор отстоит от трубы на выпуклость шва
	// у фронтальной F — по лучу от источника до центра детектора (у шва), перпендикуляр
	// до плоскости детектора короче: sqrt(F² - смещение²); у эллипса F — по вертикали
	// конус внутри трубы смотрит вдоль неё, F — по наклонному лучу до детектора у шва
	const bool shifted = scheme == Scheme::Frontal || scheme == Scheme::Elliptic;
	const float shift = (scheme == Scheme::Frontal) ? sourceShift : 0.f;
	const float bulge = WeldBulge();
	if (scheme == Scheme::Panoramic)
		focusDistance = IsConeInPipe() ? std::hypot(sourceShift, pipeDiameter / 2.f + bulge)
									   : pipeDiameter / 2.f + bulge;
	else if (isPipe)
		focusDistance = std::max(focusDistance, std::hypot(pipeDiameter + bulge, shift));

	// нижняя граница в самом поле, иначе в кадре правки F окажется ближе поверхности трубы
	const float focusMin = shifted ? std::hypot(pipeDiameter + bulge, shift) : 1.f;
	ImGui::BeginDisabled(scheme == Scheme::Panoramic);
	ImGui::DragFloat("##Фокусное расстояние#", &focusDistance, 1.f, focusMin, std::max(2000.f, focusMin),
					 "Фокусное расстояние: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();

	ImGui::DragFloat("##Ширина детектора#", &detectorWidth, 1.f, 10.f, 1000.f, "Ширина детектора: %.0f мм", ImGuiSliderFlags_AlwaysClamp);

	if (scheme == Scheme::Elliptic || scheme == Scheme::Frontal)
	{
		// от источника до трубы по перпендикуляру
		const float h = std::sqrt(std::max(focusDistance * focusDistance - shift * shift, 0.f));
		const float f = std::max(h - pipeDiameter - bulge, 0.f);
		// эллипс: не более 45° от плоскости шва (смещение ≤ f), больше 0,2f — с предупреждением на схеме;
		// фронтальная: луч на центр детектора не более 5° к плоскости шва
		const float shiftMax = (scheme == Scheme::Elliptic)
								   ? f
								   : focusDistance * std::sin(frontalAngleMax * 3.14159265f / 180.f);
		sourceShift = std::clamp(sourceShift, 0.f, shiftMax);
		// у эллипса больше 0,2f — подпись красная, как предупреждение на схеме
		const bool overShift = scheme == Scheme::Elliptic && sourceShift > 0.2f * f + 0.5f;
		if (overShift)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 60, 255));
		ImGui::DragFloat("##Смещение источника#", &sourceShift, 0.5f, 0.f, shiftMax,
						 "Смещение источника: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
		if (overShift)
			ImGui::PopStyleColor();
	}
	else if (IsConeInPipe())
	{
		// от источника до плоскости шва вдоль оси трубы
		const float shiftMax = 2.f * pipeDiameter;
		sourceShift = std::clamp(sourceShift, 0.f, shiftMax);
		ImGui::DragFloat("##Смещение источника#", &sourceShift, 1.f, 0.f, shiftMax,
						 "Смещение источника: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
	}

	const auto [thicknessMin, thicknessMax] = ThicknessRange();
	steelThickness = std::clamp(steelThickness, thicknessMin, thicknessMax);

	ImGui::DragFloat("##Толщина стали#",
					 &steelThickness,
					 0.1f,
					 thicknessMin,
					 thicknessMax,
					 isPipe ? "Толщина стенки: %.1f мм" : "Толщина стали: %.1f мм",
					 ImGuiSliderFlags_AlwaysClamp);

	ImGui::BeginDisabled(!calculatedDevices[deviceIndex].currentAdjustment ||
						 measureIndex == Measure::mAxmin);
	ImGui::SliderFloat("##Сила тока#",
					   &deviceCurrent,
					   calculatedDevices[deviceIndex].currentMinimum,
					   calculatedDevices[deviceIndex].currentMaximum,
					   "Сила тока: %.1f мА",
					   ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();

	if (!calculatedDevices[deviceIndex].currentAdjustment && measureIndex == Measure::mAxmin)
	{
		measureIndex = Measure::min;
		nameAxisY = "Время экспозиции, мин";
	}

	ImGui::SeparatorText("Номограмма экспозиции");
	ImGui::BeginDisabled(!calculatedDevices[deviceIndex].currentAdjustment);
	if (ImGui::RadioButton("мА х мин", measureIndex == Measure::mAxmin))
	{
		measureIndex = Measure::mAxmin;
		nameAxisY = "Экспозиция, мА х мин";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::RadioButton("минуты", measureIndex == Measure::min))
	{
		measureIndex = Measure::min;
		nameAxisY = "Время экспозиции, мин";
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("секунды", measureIndex == Measure::sec))
	{
		measureIndex = Measure::sec;
		nameAxisY = "Время экспозиции, с";
	}

	// при смещённом источнике луч идёт через стенку наклонно — путь в ней длиннее толщины
	ImGui::Checkbox("Фактический путь в стенке", &actualWallPath);
	const float pathFactor = actualWallPath ? WallPathFactor() : 1.f;

	// рекомендуемое напряжение U = 7·S + 100 кВ, S — просвечиваемая толщина (с учётом пути, если включён);
	// на номограмме для него — точка и пунктирная кривая, если аппарат может дать такой режим
	const XrayDevice &device = calculatedDevices[deviceIndex];
	const int recommendedU = static_cast<int>(std::lround(7.f * steelThickness * wallFactor * pathFactor + 100.f));
	float curveUMin = std::numeric_limits<float>::max();
	float curveUMax = 0.f;
	for (const Curve &curve : device.curveVector)
	{
		curveUMin = std::min(curveUMin, curve.voltage);
		curveUMax = std::max(curveUMax, curve.voltage);
	}
	const bool byVoltage = curveUMax - curveUMin >= 0.5f; // кривые разных напряжений, а не плёнок
	const bool inRange = recommendedU >= curveUMin && recommendedU <= curveUMax;
	const bool overVoltage = recommendedU > device.voltageMaximum;
	const bool overPower = recommendedU * deviceCurrent > device.electricPower;
	const bool drawRecommended = byVoltage && inRange && !overVoltage && !overPower;
	
	if (!byVoltage)
		ImGui::Text("Рекомендуемое напряжение: %d кВ (кривые не по напряжению)", recommendedU);
	else if (overVoltage)
		ImGui::TextColored(ImVec4(1.f, 0.31f, 0.24f, 1.f), "Рекомендуемое напряжение: %d кВ - превышено напряжение РА", recommendedU);
	else if (overPower)
		ImGui::TextColored(ImVec4(1.f, 0.31f, 0.24f, 1.f),
						   "Рекомендуемое напряжение: %d кВ - превышена мощность РА", recommendedU);
	else
		ImGui::Text("Рекомендуемое напряжение: %d кВ", recommendedU);

	if (ImPlot::BeginPlot(calculatedDevices[deviceIndex].name.c_str(),
						  ImVec2(-1.f, ImGui::GetContentRegionAvail().y), // до низа окна
						  ImPlotFlags_NoLegend))
	{
		ImPlot::SetupAxes("Толщина стали, мм", nameAxisY.c_str(),
						  ImPlotAxisFlags_AutoFit,
						  ImPlotAxisFlags_AutoFit);

		ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

		calculatedDevices = ExposureRecalculation(devices, focusDistance, deviceCurrent);

		std::vector<CurvesRef> visibleLines; // массив структур массивов
		visibleLines.reserve(calculatedDevices[deviceIndex].curveVector.size());

		for (const auto &curve : calculatedDevices[deviceIndex].curveVector)
		{
			if (calculatedDevices[deviceIndex].electricPower >= curve.voltage * deviceCurrent)
			{
				// по толщине стенки t нужна экспозиция как для пути t·k: кривая сжимается по толщине в k раз
				std::vector<float> xs = curve.xData;
				for (float &x : xs) // cppcheck-suppress useStlAlgorithm
					x /= pathFactor;

				ImPlot::PlotLine(curve.label.c_str(), xs.data(), curve.yData.data(), static_cast<int>(xs.size()));
				visibleLines.push_back({xs, curve.yData, ImPlot::GetLastItemColor(), curve.label});
			}
		}
		if (!visibleLines.empty())
		{
			steelThicknessMin = visibleLines.front().x.front();
			steelThicknessMax = visibleLines.back().x.back();
		}
		DrawMarkers(visibleLines, steelThickness * wallFactor);

		const auto neighbors = NeighborCurves(calculatedDevices[deviceIndex].curveVector, static_cast<float>(recommendedU));
		if (drawRecommended && neighbors)
		{
			const Curve between = InterpolatedCurve(*neighbors, pathFactor);
			auto exposure = [&](float x) { return std::pow(10.f, CurveLogExposure(between, x, 1.f)); };

			// пунктирная кривая от начала до конца промежуточной кривой: отрезки через один
			const float xMin = between.xData.front();
			const float xMax = between.xData.back();
			constexpr int dashes = 60;
			std::vector<float> dx;
			std::vector<float> dy;
			for (int i = 0; i < dashes; ++i)
			{
				const float xa = xMin + (xMax - xMin) * static_cast<float>(i) / dashes;
				const float xb = xa + 0.5f * (xMax - xMin) / dashes;
				dx.insert(dx.end(), {xa, xb});
				dy.insert(dy.end(), {exposure(xa), exposure(xb)});
			}
			// цвет текста темы: светлый на тёмной теме, тёмный на светлой
			const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			ImPlotSpec dashSpec;
			dashSpec.LineColor = accent;
			dashSpec.LineWeight = 1.5f;
			dashSpec.Flags = ImPlotLineFlags_Segments;
			ImPlot::PlotLine("##recommended", dx.data(), dy.data(), static_cast<int>(dx.size()), dashSpec);

			// точка при текущей толщине — только в пределах кривой
			const double px = steelThickness * wallFactor;
			if (px >= xMin && px <= xMax)
			{
				const double py = exposure(static_cast<float>(px));
				ImPlotSpec markerSpec;
				markerSpec.Marker = ImPlotMarker_Diamond;
				markerSpec.MarkerSize = 6.f;
				markerSpec.MarkerFillColor = accent;
				markerSpec.MarkerLineColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg); // контур — цвет фона
				ImPlot::PlotScatter("##recommended-point", &px, &py, 1, markerSpec);

				const char *unit = measureIndex == Measure::mAxmin ? "мА х мин"
								   : measureIndex == Measure::min  ? "мин"
																   : "с";
				ImPlot::Annotation(px, py, accent, ImVec2(8.f, 14.f), true,
								   measureIndex == Measure::sec ? "U=%d кВ T=%.0f %s" : "U=%d кВ T=%.1f %s",
								   recommendedU, py, unit);
			}
		}

		ImPlot::EndPlot();
	}
	ImGui::PopItemWidth();
	ImGui::EndChild();

	ImGui::SameLine();
	//ImGui::BeginChild("##scheme");
	//if (ImGui::CollapsingHeader("Схема просвечивания", ImGuiTreeNodeFlags_DefaultOpen))
		DrawScheme(ImGui::GetContentRegionAvail());
	//ImGui::EndChild();
}

std::pair<float, float> NomogramWindow::ThicknessRange() const
{
	// через две стенки номограмма считает 2t, поэтому и пределы стенки вдвое меньше;
	// у трубы стенка меньше радиуса — должен остаться внутренний канал не меньше 1 мм в радиусе
	const float wallFactor = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? 2.f : 1.f;
	const float tMin = steelThicknessMin / wallFactor;
	float tMax = steelThicknessMax / wallFactor;
	if (scheme != Scheme::Plate)
		tMax = std::min(tMax, pipeDiameter / 2.f - 1.f);
	return {tMin, std::max(tMin, tMax)};
}

float NomogramWindow::ThicknessShare() const
{
	// 0 — самая тонкая стенка в пределах ползунка, 1 — самая толстая
	const auto [tMin, tMax] = ThicknessRange();
	if (tMax - tMin < 1e-3f)
		return 1.f;
	return std::clamp((steelThickness - tMin) / (tMax - tMin), 0.f, 1.f);
}

float NomogramWindow::WeldBulge() const
{
	return weldBulgeMin + (weldBulgeMax - weldBulgeMin) * ThicknessShare();
}

float NomogramWindow::RootBulge() const
{
	return rootBulgeMax * ThicknessShare();
}

float NomogramWindow::WallPathFactor() const
{
	// центральный луч идёт к шву у детектора наклонно, если источник смещён от плоскости шва:
	// на толщину t поперёк стенки приходится сдвиг вдоль неё, путь — t·√(1 + (сдвиг/высота)²)
	float ratio = 0.f;
	if (IsConeInPipe())
	{
		// источник на оси в L от плоскости шва: на радиус r — L вдоль оси
		const float r = pipeDiameter / 2.f - steelThickness; // внутренний радиус
		if (r > 0.f)
			ratio = sourceShift / r;
	}
	else if (scheme == Scheme::Frontal || scheme == Scheme::Elliptic)
	{
		// источник над трубой, смещён на s вдоль оси; стенки на пути луча — поперёк вертикали.
		// У фронтальной F — по лучу, у эллипса — по вертикали
		const float h = (scheme == Scheme::Frontal)
							? std::sqrt(std::max(focusDistance * focusDistance - sourceShift * sourceShift, 1.f))
							: std::max(focusDistance, 1.f);
		ratio = sourceShift / h;
	}
	// панорамный аппарат в панорамной схеме и пластина — луч на шов перпендикулярен стенке
	return std::sqrt(1.f + ratio * ratio);
}

bool NomogramWindow::IsConeInPipe() const
{
	if (scheme != Scheme::Panoramic || devices.empty())
		return false;
	const XrayDevice &device = devices[deviceIndex];
	return device.beamAngleY < 360.f && device.beamAngleX == device.beamAngleY;
}

void NomogramWindow::DrawScheme(ImVec2 size)
{
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const ImVec2 p1{p0.x + size.x, p0.y + size.y};
	ImGui::Dummy(size);

	if (size.x < 100.f || size.y < 60.f)
		return;

	SchemeCanvas cl; // продольный разрез (или пластина)
	cl.dl = ImGui::GetWindowDrawList();
	cl.line = ImGui::GetColorU32(ImGuiCol_Text);
	cl.wall = ImGui::GetColorU32(ImGuiCol_FrameBgActive);

	cl.dl->PushClipRect(p0, p1, true);

	const bool isPipe = scheme != Scheme::Plate;
	const float D = pipeDiameter;
	const float R = D / 2.f;
	const float t = steelThickness;
	const float F = focusDistance;

	char textF[64];
	char textT[64];
	std::snprintf(textF, sizeof(textF), "F = %.0f мм", F);
	std::snprintf(textT, sizeof(textT), "t = %.1f мм", steelThickness);

	// всё в одном масштабе, расстояния на рисунке пропорциональны настоящим
	const float b = WeldBulge();
	// у фронтальной F — по лучу до центра детектора, по перпендикуляру ближе; у эллипса F — по вертикали
	const float shiftF = (scheme == Scheme::Frontal) ? sourceShift : 0.f;
	const float H = std::sqrt(std::max(F * F - shiftF * shiftF, 0.f)); // до плоскости детектора
	// от ближней к источнику поверхности объекта до детектора: у трубы — De и выпуклость
	// нижнего шва, у пластины — толщина и обратный валик (основной валик к источнику)
	const float rootB = RootBulge();
	const float depth = isPipe ? D + b : t + rootB;
	const float aReal = std::max(H - depth, 0.f); // от источника до объекта
	const float filmHalf = detectorWidth / 2.f;
	const float plateLen = std::max({12.f * t, 60.f, 1.3f * detectorWidth});

	// конус внутри трубы: источник на оси, на coneShift от плоскости шва, пучок вдоль трубы
	const bool cone = IsConeInPipe();
	const float coneShift = cone ? sourceShift : 0.f;
	const float pipeLen = std::max({1.6f * D, 1.3f * detectorWidth, 2.f * (coneShift + 0.25f * D)});
	// далёкий источник рисуем ближе (у труб не дальше 1,2·De, у пластины — 0,8 её длины),
	// размеры и пучок тогда с разрывом
	const float aShown = std::min(aReal, isPipe ? 1.2f * D : 0.8f * plateLen);
	const bool broken = aReal > aShown + 0.5f;

	// смещение источника на рисунке: у эллипса — в масштабе показанного f, чтобы луч через
	// верхний шов приходил в то же место детектора; у фронтальной сохраняется угол луча на центр
	// детектора; sep — расхождение изображений швов
	const float shiftX = (scheme == Scheme::Frontal) ? sourceShift * (aShown + D + b) / (aReal + D + b)
						 : (aReal > 0.f)			 ? sourceShift * aShown / aReal
													 : sourceShift;
	const float sep = aReal > 0.f ? sourceShift * (D + b) / aReal : 0.f;

	ImVec2 lMin, lMax; // границы видов в мм
	ImVec2 cMin, cMax;
	switch (scheme)
	{
	case Scheme::Panoramic:
		lMin = {-pipeLen / 2.f, -R - b};
		lMax = {pipeLen / 2.f, R + b};
		cMin = {-R - b, -R - b};
		cMax = {R + b, R + b};
		break;
	case Scheme::Frontal:
	case Scheme::Elliptic:
		lMin = {-pipeLen / 2.f, 0.f};
		lMax = {pipeLen / 2.f, aShown + D + b};
		lMax.x = std::max(lMax.x, shiftX);
		cMin = {-R - b, 0.f};
		cMax = {R + b, aShown + D + b};
		break;
	default:
		lMin = {-plateLen / 2.f, 0.f};
		lMax = {plateLen / 2.f, aShown + depth};
		break;
	}

	const float marginX = ImGui::CalcTextSize("F = 2000 мм").x + 20.f;
	const float marginY = ImGui::GetTextLineHeight() + 8.f;
	// виды один над другим: сверху продольный разрез, снизу сечение
	const float splitY = isPipe ? p0.y + size.y * 0.5f : p1.y;

	// у каждого вида свой фон со скруглёнными углами, как у виджетов, между видами — зазор
	const float viewGap = isPipe ? ImGui::GetStyle().ItemSpacing.y / 2.f : 0.f;
	const ImVec2 longMax{p1.x, splitY - viewGap};
	const ImVec2 crossMin{p0.x, splitY + viewGap};
	const ImU32 viewBg = ImGui::GetColorU32(ImGuiCol_FrameBg);
	const float rounding = ImGui::GetStyle().FrameRounding;
	cl.dl->AddRectFilled(p0, longMax, viewBg, rounding);
	if (isPipe)
		cl.dl->AddRectFilled(crossMin, p1, viewBg, rounding);

	// у эллипса и фронтальной над источником размер смещения, над сечением фронтальной —
	// число экспозиций
	const float marginTop = marginY + ((scheme == Scheme::Elliptic || scheme == Scheme::Frontal)
										   ? ImGui::GetTextLineHeight() + 14.f
										   : 0.f);
	const float marginTopCross = marginY + (scheme == Scheme::Frontal ? ImGui::GetTextLineHeight() : 0.f);

	const ImVec2 lsMin{p0.x + marginX, p0.y + marginTop};
	const ImVec2 lsMax{p1.x - marginX, splitY - marginY};
	const ImVec2 csMin{p0.x + marginX, splitY + marginTopCross};
	const ImVec2 csMax{p1.x - marginX, p1.y - marginY};

	cl.scale = FitScale(lMin, lMax, lsMin, lsMax);
	if (isPipe)
		cl.scale = std::min(cl.scale, FitScale(cMin, cMax, csMin, csMax));
	CenterView(cl, lMin, lMax, lsMin, lsMax);

	SchemeCanvas cc = cl; // поперечное сечение
	CenterView(cc, cMin, cMax, csMin, csMax);

	const float gap = cl.Mm(12.f); // отступ размерных линий от объекта

	// Пучок аппарата рисуем первым, под объектом: X — на продольном разрезе, Y — в сечении
	const XrayDevice &device = devices[deviceIndex];
	const bool panoramicDevice = device.beamAngleY >= 360.f;
	const float sourceX = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? shiftX : coneShift;
	// от источника до детектора на рисунке: до него пучок ровный, за ним гаснет
	const float beamFull = cl.Px(cone						   ? std::hypot(coneShift, R + b)
								 : scheme == Scheme::Panoramic ? R + b
								 : isPipe					   ? aShown + D + b
															   : aShown + depth);
	const float beamFade = std::max(40.f, 0.35f * beamFull);

	// При разрыве f пучок тоже с разрывом, как на рис. 11 ISO 17636-1: у источника — короткий
	// конус с настоящим углом от нарисованного источника, ниже разрыва — конус с настоящим углом
	// от источника на настоящем расстоянии (за рамкой), поэтому у трубы ширина пучка верная
	const float breakY = 0.5f * aShown; // на одной высоте с разрывом размера f
	auto drawBeam = [&](const SchemeCanvas &c, ImVec2 clipMin, ImVec2 clipMax, float xShown, float xReal,
						float angleDeg)
	{
		if (!broken || angleDeg >= 179.f)
		{
			Beam(c, xShown, 0.f, pi / 2.f, angleDeg, beamFull, beamFade);
			return;
		}
		const float gapY = c.P(0.f, breakY).y;
		c.dl->PushClipRect(clipMin, {clipMax.x, gapY - 3.f}, true);
		Beam(c, xShown, 0.f, pi / 2.f, angleDeg, beamFull, beamFade);
		c.dl->PopClipRect();

		const float vy = aShown - aReal; // настоящий источник, выше рамки
		c.dl->PushClipRect({clipMin.x, gapY + 3.f}, clipMax, true);
		Beam(c, xReal, vy, pi / 2.f, angleDeg, c.Px(aShown + depth - vy), beamFade);
		c.dl->PopClipRect();
	};

	cl.dl->PushClipRect(p0, longMax, true);
	if (cone)
		Beam(cl, sourceX, 0.f, pi, device.beamAngleX, beamFull, beamFade); // вдоль трубы к шву
	else
	{
		if (scheme != Scheme::Panoramic)
			drawBeam(cl, p0, longMax, sourceX, (scheme == Scheme::Plate) ? 0.f : sourceShift, device.beamAngleX);
		else
			Beam(cl, sourceX, 0.f, pi / 2.f, device.beamAngleX, beamFull, beamFade);
		// панорамный аппарат излучает по кругу — на продольном разрезе и вверх
		if (panoramicDevice)
			Beam(cl, sourceX, 0.f, -pi / 2.f, device.beamAngleX, beamFull, beamFade);
	}
	cl.dl->PopClipRect();

	if (isPipe)
	{
		// конус вдоль трубы в сечении освещает весь периметр
		cc.dl->PushClipRect(crossMin, p1, true);
		if (scheme != Scheme::Panoramic)
			drawBeam(cc, crossMin, p1, 0.f, 0.f, device.beamAngleY);
		else
			Beam(cc, 0.f, 0.f, pi / 2.f, cone ? 360.f : device.beamAngleY, beamFull, beamFade);
		cc.dl->PopClipRect();
	}

	// Предупреждения — по настоящим размерам, а не по рисунку; выводятся все, столбиком
	std::vector<std::string> warnings;
	{
		if (scheme == Scheme::Elliptic && sourceShift > 0.2f * aReal + 0.5f)
			warnings.emplace_back("Выход за пределы допустимого смещения");

		const float shift = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? sourceShift : 0.f;
		// для конуса — угол от оси трубы до ближнего к источнику края детектора
		const float need = cone ? std::atan2(R + b, coneShift - filmHalf) * 180.f / pi
								: std::atan((filmHalf + shift) / std::max(H, 1.f)) * 180.f / pi;
		if (scheme == Scheme::Panoramic && !panoramicDevice && !cone)
			warnings.emplace_back("Аппарат не панорамный");
		else if (scheme != Scheme::Elliptic && need > device.beamAngleX / 2.f) // у эллипса не проверяем
			warnings.emplace_back("Пучок не накрывает детектор");
		else if (scheme == Scheme::Elliptic)
		{
			// изображение трубы — между касательными от источника, по настоящим размерам
			const float dist = aReal + R;
			const float shadowHalf = (aReal + D + b) * R / std::sqrt(std::max(dist * dist - R * R, 1e-3f));
			if (aReal <= 0.f || shadowHalf > ellipseFilmLength / 2.f)
				warnings.emplace_back("Изображение не помещается на детектор");
		}
		else if (scheme == Scheme::Frontal)
		{
			// в сечении: края изогнутого детектора (периметр / N) от источника над трубой
			const float span = pi / static_cast<float>(FrontalExposures(D, steelThickness, aReal));
			const float edgeX = (R + b) * std::sin(span);
			const float edgeY = R + aReal + (R + b) * std::cos(span); // ниже источника
			if (std::atan2(edgeX, edgeY) * 180.f / pi > device.beamAngleY / 2.f)
				warnings.emplace_back("Пучок не накрывает края детектора по периметру");
		}
	}

	switch (scheme)
	{
	case Scheme::Panoramic:
	{
		HatchRect(cl, -pipeLen / 2.f, -R, pipeLen / 2.f, -R + t, true);
		HatchRect(cl, -pipeLen / 2.f, R - t, pipeLen / 2.f, R, true);
		for (float ex : {-pipeLen / 2.f, pipeLen / 2.f})
			BreakLine(cl, ex, -R, R); // обрыв трубы
		AxisLine(cl, -pipeLen / 2.f - gap, 0.f, pipeLen / 2.f + gap, 0.f);
		JointLine(cl, 0.f, -R + t, R - t);
		Weld(cl, 0.f, -R, -R + t, b);
		Weld(cl, 0.f, R, R - t, b);
		FlatFilm(cl, -filmHalf, filmHalf, -R - b, -1.f);
		FlatFilm(cl, -filmHalf, filmHalf, R + b, 1.f);
		Source(cl, sourceX, 0.f);
		DimV(cl, pipeLen / 2.f + gap, -R, -R + t, pipeLen / 2.f, pipeLen / 2.f, textT, false, true);
		if (cone)
		{
			// F — по наклонному лучу до шва у детектора, подпись у середины луча
			Ray(cl, sourceX, 0.f, 0.f, R + b);
			const ImVec2 m = cl.P(sourceX / 2.f, (R + b) / 2.f);
			cl.dl->AddText({m.x + 6.f, m.y}, cl.line, textF);

			if (coneShift >= 0.5f)
			{
				char textS[64];
				std::snprintf(textS, sizeof(textS), "%.0f мм", coneShift);
				// в верхней половине трубы, чтобы не пересекаться с лучом F, идущим вниз
				DimH(cl, -R * 0.5f, sourceX, 0.f, 0.f, -R + t, textS);
			}
		}

		// f — от источника до внутренней поверхности стенки
		char textFi[64];
		std::snprintf(textFi, sizeof(textFi), "f = %.0f мм", R - t);
		DimV(cl, -pipeLen / 2.f - gap, 0.f, R - t, sourceX, -pipeLen / 2.f, textFi, true);

		HatchRing(cc, 0.f, 0.f, R - t, R);
		ArcFilm(cc, 0.f, 0.f, R + b, 0.f, 2.f * pi);
		AxisLine(cc, -R - gap, 0.f, R + gap, 0.f);
		AxisLine(cc, 0.f, -R - gap, 0.f, R + gap);
		Source(cc, 0.f, 0.f);
		break;
	}
	case Scheme::Frontal:
	case Scheme::Elliptic:
	{
		const bool elliptic = scheme == Scheme::Elliptic;
		const float top = aShown;
		const float bottom = aShown + D;
		const float filmY = bottom + b; // детектор за выпуклостью нижнего шва
		const float sx = shiftX;

		HatchRect(cl, -pipeLen / 2.f, top, pipeLen / 2.f, top + t, true);
		HatchRect(cl, -pipeLen / 2.f, bottom - t, pipeLen / 2.f, bottom, true);
		for (float ex : {-pipeLen / 2.f, pipeLen / 2.f})
			BreakLine(cl, ex, top, bottom); // обрыв трубы
		AxisLine(cl, -pipeLen / 2.f - gap, top + R, pipeLen / 2.f + gap, top + R);
		JointLine(cl, 0.f, top + t, bottom - t);
		Weld(cl, 0.f, top, top + t, b);
		Weld(cl, 0.f, bottom, bottom - t, b);
		FlatFilm(cl, -filmHalf, filmHalf, filmY, 1.f);
		if (elliptic)
		{
			// луч через верхний шов приходит на детектор левее нижнего — получается эллипс
			Ray(cl, sx, 0.f, top > 0.f ? -sep : 0.f, filmY);
			Ray(cl, sx, 0.f, 0.f, filmY);

			// изображения швов: верхний увеличен в (f + De + 3) / f раз — по настоящим размерам
			const float weldHalf = std::max(t * 0.7f, cl.Mm(3.f));
			const float zoom = aReal > 0.f ? (aReal + D + b) / aReal : 1.f;
			WeldImage(cl, top > 0.f ? -sep : 0.f, filmY, weldHalf * zoom);
			WeldImage(cl, 0.f, filmY, weldHalf);
		}
		else
			Ray(cl, sx, 0.f, 0.f, filmY); // наклонный луч через оцениваемый шов на центр детектора
		Source(cl, sx, 0.f);
		DimV(cl, pipeLen / 2.f + gap, bottom - t, bottom, pipeLen / 2.f, pipeLen / 2.f, textT, false, true);
		// f — у эллипса до ближайшей поверхности трубы, у фронтальной — до внутренней
		// поверхности стенки у детектора
		char textA[64];
		std::snprintf(textA, sizeof(textA), "f = %.0f мм", elliptic ? aReal : aReal + D - t);
		DimV(cl, lMin.x - gap, 0.f, elliptic ? top : bottom - t, sx, -pipeLen / 2.f, textA, true, false,
			 broken ? breakY : noBreak);
		if (!elliptic && aReal >= 0.5f)
		{
			// у фронтальной ещё и расстояние до наружной поверхности трубы — справа
			char textP[64];
			std::snprintf(textP, sizeof(textP), "f' = %.0f мм", aReal);
			DimV(cl, lMax.x + gap, 0.f, top, sx, pipeLen / 2.f, textP, false, false, broken ? breakY : noBreak);
		}
		if (sourceShift >= 0.5f)
		{
			char textS[64];
			if (elliptic)
				std::snprintf(textS, sizeof(textS), "%.0f мм", sourceShift);
			else
				std::snprintf(textS, sizeof(textS), "%.0f мм (%.1f°)", sourceShift,
							  std::asin(sourceShift / std::max(F, 1.f)) * 180.f / pi);
			// от плоскости шва (продолжение линии стыка) до источника
			DimH(cl, -gap, 0.f, sx, top, 0.f, textS);
		}

		HatchRing(cc, 0.f, top + R, R - t, R);
		AxisLine(cc, -R - gap, top + R, R + gap, top + R);
		AxisLine(cc, 0.f, 0.f, 0.f, bottom + gap);
		if (elliptic)
		{
			const float half = ellipseFilmLength / 2.f;
			FlatFilm(cc, -half, half, filmY, 1.f);

			// касательные от источника к трубе до детектора — границы изображения трубы;
			// угол — от настоящего источника, при разрыве линии прерываются там же, где пучок:
			// выше — от нарисованного источника, ниже — от настоящего (за рамкой)
			const float dist = aReal + R; // от настоящего источника до оси трубы
			if (dist > R)
			{
				const float tanT = R / std::sqrt(dist * dist - R * R);
				if (broken)
				{
					const float gapY = cc.P(0.f, breakY).y;
					const float vy = aShown - aReal; // настоящий источник, выше рамки
					cc.dl->PushClipRect(crossMin, {p1.x, gapY - 3.f}, true);
					for (float side : {-1.f, 1.f})
						Ray(cc, 0.f, 0.f, side * breakY * tanT, breakY);
					cc.dl->PopClipRect();
					cc.dl->PushClipRect({crossMin.x, gapY + 3.f}, p1, true);
					for (float side : {-1.f, 1.f})
						Ray(cc, 0.f, vy, side * (filmY - vy) * tanT, filmY);
					cc.dl->PopClipRect();
				}
				else
					for (float side : {-1.f, 1.f})
						Ray(cc, 0.f, 0.f, side * filmY * tanT, filmY);
			}

			// в правом верхнем углу сечения — вид на детектор с эллипсом
			const float regionW = p1.x - crossMin.x;
			const float regionH = p1.y - crossMin.y;
			const float insetW = 0.38f * regionW;
			const float insetH = std::min(insetW * detectorWidth / ellipseFilmLength, 0.5f * regionH);
			const ImVec2 insetMin{p1.x - 6.f - insetW, crossMin.y + 6.f};
			EllipseDetectorView(cc, insetMin, {p1.x - 6.f, insetMin.y + insetH}, D, steelThickness, aReal, b,
								sourceShift, ellipseFilmLength, detectorWidth);

			// под ним — коэффициент для размеров дефектов шва со стороны источника:
			// α = (f + S) / (f + S + D), f — до поверхности контролируемого участка
			const float alpha = (aReal + steelThickness) / (aReal + steelThickness + D);
			char textAlpha[64];
			std::snprintf(textAlpha, sizeof(textAlpha), "Коэффициент размера дефекта α = %.2f", alpha);
			const ImVec2 ts = ImGui::CalcTextSize(textAlpha);
			cc.dl->AddText({p1.x - 6.f - ts.x, insetMin.y + insetH + 4.f}, cc.line, textAlpha);
		}
		else
		{
			// число экспозиций — по настоящим размерам; детектор — периметр / N
			const int exposures = FrontalExposures(D, steelThickness, aReal);
			const float span = pi / static_cast<float>(exposures); // полуугол изогнутого детектора
			ArcFilm(cc, 0.f, top + R, R + b, pi / 2.f - span, pi / 2.f + span);

			char textN[96];
			std::snprintf(textN, sizeof(textN), "Количество экспозиций по ГОСТ 7512-82 = %d", exposures);
			cc.dl->AddText({p0.x + 6.f, crossMin.y + 4.f}, cc.line, textN); // левый верхний угол сечения
		}
		Source(cc, 0.f, 0.f);
		break;
	}
	default:
	{
		const float top = aShown;
		const float bottom = aShown + t;

		const float filmY = bottom + rootB; // детектор за обратным валиком

		HatchRect(cl, -plateLen / 2.f, top, plateLen / 2.f, bottom, true);
		for (float ex : {-plateLen / 2.f, plateLen / 2.f})
			BreakLine(cl, ex, top, bottom);
		Weld(cl, 0.f, top, bottom, b);				   // валиком к источнику
		if (rootB > 0.05f)
			RootBead(cl, 0.f, bottom, 0.15f * t, rootB, 1.f); // обратный валик к детектору
		FlatFilm(cl, -filmHalf, filmHalf, filmY, 1.f);
		AxisLine(cl, 0.f, 0.f, 0.f, filmY + gap);
		Source(cl, 0.f, 0.f);
		DimV(cl, plateLen / 2.f + gap, top, bottom, plateLen / 2.f, plateLen / 2.f, textT, false, true);
		// f — до ближайшей поверхности пластины
		char textA[64];
		std::snprintf(textA, sizeof(textA), "f = %.0f мм", aReal);
		DimV(cl, -plateLen / 2.f - gap, 0.f, top, 0.f, -plateLen / 2.f, textA, true, false,
			 broken ? breakY : noBreak);
		break;
	}
	}

	if (scheme == Scheme::Frontal || scheme == Scheme::Elliptic)
	{
		char info[96];
		std::snprintf(info, sizeof(info), "Просвечиваемая толщина 2t = %.1f мм", 2.f * steelThickness);
		cl.dl->AddText({p0.x + 6.f, p0.y + 4.f}, cl.line, info); // левый верхний угол продольного разреза
	}

	// фактический путь центрального луча в металле — в левом нижнем углу продольного разреза
	{
		const float k = WallPathFactor();
		const float nominal = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? 2.f * t : t;
		char textPath[96];
		std::snprintf(textPath, sizeof(textPath), "Путь в металле: %.1f мм (×%.2f)", nominal * k, k);
		cl.dl->AddText({p0.x + 6.f, longMax.y - ImGui::GetTextLineHeight() - 4.f}, cl.line, textPath);
	}

	// предупреждения в правом нижнем углу, снизу вверх
	float warningY = p1.y - 4.f;
	for (const std::string &w : warnings)
	{
		const ImVec2 ts = ImGui::CalcTextSize(w.c_str());
		warningY -= ts.y;
		cl.dl->AddText({p1.x - ts.x - 6.f, warningY}, cl.source, w.c_str());
	}

	cl.dl->PopClipRect();
}

std::vector<XrayDevice> NomogramWindow::ExposureRecalculation(const std::vector<XrayDevice> &deviceVector,
															  float distance, float current) const
{
	auto result = deviceVector;
	auto &device = result[deviceIndex];

	float factor = static_cast<float>(std::pow((distance / deviceVector[deviceIndex].focusDistanceDefault), 2));

	if (!result[deviceIndex].currentAdjustment)
		current = 1.f;

	switch (measureIndex)
	{
	case Measure::min:
		factor /= current; // мА х мин переводим в минуты
		break;
	case Measure::sec:
		factor /= current / 60.f; // переводим в секунды
		break;
	default:
		break;
	}

	for (auto &curve : device.curveVector)
	{
		for (auto &y : curve.yData)
		{ // cppcheck-suppress useStlAlgorithm
			y *= factor;
		}
	}

	return result;
}

// AI generated
void NomogramWindow::DrawMarkers(const std::vector<CurvesRef> &curves, float thickness) const
{
	if (curves.empty())
		return;
	if (!std::isfinite(thickness))
		return;

	ImPlotSpec lineSpec;
	lineSpec.LineColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImPlot::PlotInfLines("##thickness-line", &thickness, 1, lineSpec);

	for (const auto &line : curves)
	{
		if (line.x.empty() || line.y.empty())
			continue;

		const auto &xs = line.x;
		const auto &ys = line.y;

		if (xs.size() < 2 || xs.size() != ys.size())
			continue;

		for (size_t i = 0; i + 1 < xs.size(); ++i)
		{
			const float x0 = xs.at(i);
			const float x1 = xs.at(i + 1);
			const float minX = std::min(x0, x1);
			const float maxX = std::max(x0, x1);

			if (thickness < minX || thickness > maxX)
				continue;

			const float delta = x1 - x0;
			if (std::fabs(delta) <= std::numeric_limits<float>::epsilon())
				continue;

			const float t = std::clamp((thickness - x0) / delta, 0.0f, 1.0f);

			double px = {thickness};

			const float y0 = ys.at(i);
			const float y1 = ys.at(i + 1);
			// интерполяция по логарифму для логарифмической шкалы
			double py = std::pow(10.0, std::log10(y0) + t * (std::log10(y1) - std::log10(y0)));

			const ImVec4 fillColor = line.color;
			const ImVec4 outlineColor{0.05f, 0.05f, 0.05f, 1.0f};

			std::string markerLabel = "##thickness-marker-" + line.label + "-" + std::to_string(i);

			ImPlotSpec markerSpec;
			markerSpec.LineWeight = 2.0f;
			markerSpec.Marker = ImPlotMarker_Diamond;
			markerSpec.MarkerSize = 6.0f;
			markerSpec.MarkerLineColor = outlineColor;
			markerSpec.MarkerFillColor = fillColor;
			ImPlot::PlotScatter(markerLabel.c_str(), &px, &py, 1, markerSpec);

			switch (measureIndex)
			{
			case Measure::mAxmin:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f %s  T=%.1f %s", line.label.c_str(), px, "мм", py, "мА х мин");
				break;
			case Measure::min:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f %s  T=%.1f %s", line.label.c_str(), px, "мм", py, "мин");
				break;
			case Measure::sec:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f %s  T=%.0f %s", line.label.c_str(), px, "мм", py, "с");
				break;
			default:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f %s  T=%.1f %s", line.label.c_str(), px, "мм", py, "мА");
				break;
			}
			break;
		}
	}
}