#include "nomogramWindow.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <limits>
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
	if (scheme == Scheme::Panoramic)
		focusDistance = IsConeInPipe() ? std::hypot(sourceShift, pipeDiameter / 2.f + weldBulge)
									   : pipeDiameter / 2.f + weldBulge;
	else if (isPipe)
		focusDistance = std::max(focusDistance, std::hypot(pipeDiameter + weldBulge, shift));

	// нижняя граница в самом поле, иначе в кадре правки F окажется ближе поверхности трубы
	const float focusMin = shifted ? std::hypot(pipeDiameter + weldBulge, shift) : 1.f;
	ImGui::BeginDisabled(scheme == Scheme::Panoramic);
	ImGui::DragFloat("##Фокусное расстояние#", &focusDistance, 1.f, focusMin, std::max(2000.f, focusMin),
					 "Фокусное расстояние: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();

	ImGui::DragFloat("##Ширина детектора#", &detectorWidth, 1.f, 10.f, 1000.f, "Ширина детектора: %.0f мм", ImGuiSliderFlags_AlwaysClamp);

	if (scheme == Scheme::Elliptic || scheme == Scheme::Frontal)
	{
		// от источника до трубы по перпендикуляру
		const float h = std::sqrt(std::max(focusDistance * focusDistance - shift * shift, 0.f));
		const float f = std::max(h - pipeDiameter - weldBulge, 0.f);
		// эллипс: не более 0,2f; фронтальная: луч на центр детектора не более 5° к плоскости шва
		const float shiftMax = (scheme == Scheme::Elliptic)
								   ? 0.2f * f
								   : focusDistance * std::sin(frontalAngleMax * 3.14159265f / 180.f);
		sourceShift = std::clamp(sourceShift, 0.f, shiftMax);
		ImGui::DragFloat("##Смещение источника#", &sourceShift, 0.5f, 0.f, shiftMax,
						 "Смещение источника: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
	}
	else if (IsConeInPipe())
	{
		// от источника до плоскости шва вдоль оси трубы
		const float shiftMax = 2.f * pipeDiameter;
		sourceShift = std::clamp(sourceShift, 0.f, shiftMax);
		ImGui::DragFloat("##Смещение источника#", &sourceShift, 1.f, 0.f, shiftMax,
						 "Смещение источника: %.0f мм", ImGuiSliderFlags_AlwaysClamp);
	}

	if (steelThickness > steelThicknessMax / wallFactor)
		steelThickness = steelThicknessMax / wallFactor;

	if (steelThickness < steelThicknessMin / wallFactor)
		steelThickness = steelThicknessMin / wallFactor;

	ImGui::DragFloat("##Толщина стали#",
					 &steelThickness,
					 0.1f,
					 steelThicknessMin / wallFactor,
					 steelThicknessMax / wallFactor,
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

	// высоты постоянные: если окно ниже, вкладка прокручивается
	DrawScheme(ImVec2(ImGui::GetContentRegionAvail().x, 24.f * ImGui::GetFontSize()));

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

	if (ImPlot::BeginPlot(calculatedDevices[deviceIndex].name.c_str(),
						  ImVec2(-1, 30.f * ImGui::GetFontSize()),
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
				ImPlot::PlotLine(curve.label.c_str(),
								 curve.xData.data(),
								 curve.yData.data(),
								 static_cast<int>(curve.xData.size()));

				visibleLines.push_back({curve.xData, curve.yData,
										ImPlot::GetLastItemColor(), curve.label});
			}
		}
		if (!visibleLines.empty())
		{
			steelThicknessMin = visibleLines.front().x.front();
			steelThicknessMax = visibleLines.back().x.back();
		}
		DrawMarkers(visibleLines, steelThickness * wallFactor);

		ImPlot::EndPlot();
	}
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

	cl.dl->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_FrameBg));
	cl.dl->PushClipRect(p0, p1, true);

	const bool isPipe = scheme != Scheme::Plate;
	const float D = pipeDiameter;
	const float R = D / 2.f;
	const float t = isPipe ? std::min(steelThickness, R * 0.45f) : steelThickness; // на рисунке
	const float F = focusDistance;

	char textF[64];
	char textT[64];
	std::snprintf(textF, sizeof(textF), "F = %.0f мм", F);
	std::snprintf(textT, sizeof(textT), "t = %.1f мм", steelThickness);

	// всё в одном масштабе, расстояния на рисунке пропорциональны настоящим
	const float b = weldBulge;
	// у фронтальной F — по лучу до центра детектора, по перпендикуляру ближе; у эллипса F — по вертикали
	const float shiftF = (scheme == Scheme::Frontal) ? sourceShift : 0.f;
	const float H = std::sqrt(std::max(F * F - shiftF * shiftF, 0.f)); // до плоскости детектора
	// от ближней к источнику поверхности объекта до детектора: у трубы — De и выпуклость
	// нижнего шва, у пластины валик шва к источнику, детектор вплотную
	const float depth = isPipe ? D + b : t;
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
	const float splitX = isPipe ? p0.x + size.x * 0.6f : p1.x;

	// у эллипса над источником размер смещения
	const float marginTop = marginY + ((scheme == Scheme::Elliptic || scheme == Scheme::Frontal)
										   ? ImGui::GetTextLineHeight() + 14.f
										   : 0.f);

	const ImVec2 lsMin{p0.x + marginX, p0.y + marginTop};
	const ImVec2 lsMax{splitX - marginX, p1.y - marginY};
	const ImVec2 csMin{splitX + marginX / 2.f, p0.y + marginTop};
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

	cl.dl->PushClipRect(p0, {splitX, p1.y}, true);
	if (cone)
		Beam(cl, sourceX, 0.f, pi, device.beamAngleX, beamFull, beamFade); // вдоль трубы к шву
	else
	{
		if (scheme != Scheme::Panoramic)
			drawBeam(cl, p0, {splitX, p1.y}, sourceX, (scheme == Scheme::Plate) ? 0.f : sourceShift, device.beamAngleX);
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
		cc.dl->PushClipRect({splitX, p0.y}, p1, true);
		if (scheme != Scheme::Panoramic)
			drawBeam(cc, {splitX, p0.y}, p1, 0.f, 0.f, device.beamAngleY);
		else
			Beam(cc, 0.f, 0.f, pi / 2.f, cone ? 360.f : device.beamAngleY, beamFull, beamFade);
		cc.dl->PopClipRect();
	}

	// Накрывает ли пучок детектор — по настоящим размерам, а не по рисунку
	std::string warning;
	{
		const float shift = (scheme == Scheme::Frontal || scheme == Scheme::Elliptic) ? sourceShift : 0.f;
		// для конуса — угол от оси трубы до ближнего к источнику края детектора
		const float need = cone ? std::atan2(R + b, coneShift - filmHalf) * 180.f / pi
								: std::atan((filmHalf + shift) / std::max(H, 1.f)) * 180.f / pi;
		if (scheme == Scheme::Panoramic && !panoramicDevice && !cone)
			warning = "Аппарат не панорамный";
		else if (need > device.beamAngleX / 2.f)
			warning = "Пучок не накрывает детектор";
		else if (scheme == Scheme::Frontal)
		{
			// в сечении: края изогнутого детектора (периметр / N) от источника над трубой
			const float span = pi / static_cast<float>(FrontalExposures(D, steelThickness, aReal));
			const float edgeX = (R + b) * std::sin(span);
			const float edgeY = R + aReal + (R + b) * std::cos(span); // ниже источника
			if (std::atan2(edgeX, edgeY) * 180.f / pi > device.beamAngleY / 2.f)
				warning = "Пучок не накрывает края детектора по периметру";
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
			const float half = 0.6f * D;
			FlatFilm(cc, -half, half, filmY, 1.f);
		}
		else
		{
			// число экспозиций — по настоящим размерам; детектор — периметр / N
			const int exposures = FrontalExposures(D, steelThickness, aReal);
			const float span = pi / static_cast<float>(exposures); // полуугол изогнутого детектора
			ArcFilm(cc, 0.f, top + R, R + b, pi / 2.f - span, pi / 2.f + span);

			char textN[96];
			std::snprintf(textN, sizeof(textN), "Экспозиций по ГОСТ 7512-82: %d", exposures);
			const ImVec2 ts = ImGui::CalcTextSize(textN);
			const ImVec2 under = cc.P(0.f, filmY);
			cc.dl->AddText({under.x - ts.x / 2.f, under.y + filmPx + 4.f}, cc.line, textN);
		}
		Source(cc, 0.f, 0.f);
		break;
	}
	default:
	{
		const float top = aShown;
		const float bottom = aShown + t;

		const float filmY = bottom; // валик шва к источнику, детектор вплотную

		HatchRect(cl, -plateLen / 2.f, top, plateLen / 2.f, bottom, true);
		for (float ex : {-plateLen / 2.f, plateLen / 2.f})
			BreakLine(cl, ex, top, bottom);
		Weld(cl, 0.f, top, bottom, b); // валиком к источнику
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
		cl.dl->AddText({p0.x + 6.f, p1.y - ImGui::GetTextLineHeight() - 4.f}, cl.line, info);
	}

	// граница между продольным разрезом и сечением
	if (isPipe)
		cl.dl->AddLine({splitX, p0.y}, {splitX, p1.y}, ImGui::GetColorU32(ImGuiCol_Separator), 2.f);

	if (!warning.empty())
	{
		const ImVec2 ts = ImGui::CalcTextSize(warning.c_str());
		cl.dl->AddText({p1.x - ts.x - 6.f, p1.y - ts.y - 4.f}, cl.source, warning.c_str());
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