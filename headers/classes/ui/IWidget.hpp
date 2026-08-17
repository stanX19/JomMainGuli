#pragma once

namespace ui {

class IWidget {
public:
	virtual ~IWidget() = default;

	bool tickAndDraw() {
		const bool changed = update();
		draw();
		return changed;
	}

	virtual bool update() = 0;
	virtual void draw() = 0;
};

} // namespace ui
