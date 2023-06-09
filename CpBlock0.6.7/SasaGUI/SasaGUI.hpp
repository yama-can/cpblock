﻿#include <Siv3D.hpp> // OpenSiv3D v0.6.7

namespace SasaGUI
{
	namespace detail
	{
		class WindowImpl;

		class GUIImpl;
	}

	enum class WindowFlag : int32
	{
		/// <summary>
		/// 通常のウィンドウ
		/// </summary>
		None = 0,
		/// <summary>
		/// タイトルバーを表示しない
		/// </summary>
		NoTitlebar = 1 << 0,
		/// <summary>
		/// サイズ変更をしない
		/// </summary>
		NoResize = 1 << 1,
		/// <summary>
		/// 移動をしない
		/// </summary>
		NoMove = 1 << 2,
		/// <summary>
		/// 背景を表示しない
		/// </summary>
		NoBackground = 1 << 3,
		/// <summary>
		/// スクロールバーを表示しない
		/// </summary>
		NoScrollbar = 1 << 4,
		/// <summary>
		/// 自動リサイズ
		/// </summary>
		AutoResize = 1 << 5,
		/// <summary>
		/// 常に手前に表示
		/// </summary>
		AlwaysForeground = 1 << 6,
		/// <summary>
		/// フォーカスしない
		/// </summary>
		NoFocus = 1 << 7,
		/// <summary>
		/// ウィンドウを隠す
		/// </summary>
		Hide = 1 << 8,
		/// <summary>
		/// 無効
		/// </summary>
		Disable = 1 << 9,

		Debug = 1 << 31
	};
	DEFINE_BITMASK_OPERATORS(WindowFlag);

	struct Window
	{
		String displayName;

		Rect rect;

		WindowFlag flags;

		Size maxSize{ std::numeric_limits<Size::value_type>::max(), std::numeric_limits<Size::value_type>::max() };

		int32 space = 5;

		int32 padding = 10;

		bool defined = false;

		Rect lineRect{ padding, padding, 0, 0 };

		bool requestMoveToFront = false;

		Font font = SimpleGUI::GetFont();

		bool sameLine = false;
	};

	class IControl
	{
	public:

		virtual Size computeSize() const = 0;

		virtual void update(Rect rect, Optional<Vec2> cursorPos) = 0;

		virtual void draw() const = 0;

		virtual ~IControl() { };
	};

	class GUIManager
	{
	public:

		GUIManager();

		static GUIManager& FromAddon(StringView name);

	public:

		// Frame

		void frameBegin();

		void frameEnd();

		// Window

		void windowBegin(StringView name, WindowFlag flags = WindowFlag::None);

		void windowEnd();

		const Window& getDefaultWindow() const;

		const Window& getCurrentWindow() const;

		// Controls

		void sameLine();

		void dummy(Size size);

		bool button(StringView label);

		TextEditState& simpleTextBox(StringView id, double width = 200, const Optional<size_t>& maxChars = unspecified);

		void label(StringView text, ColorF color = Palette::Black);

		void image(Texture texture, ColorF diffuse = Palette::White);

		void image(TextureRegion texture, ColorF diffuse = Palette::White);

		bool checkbox(bool& checked, StringView label = U"");

		bool radiobutton(bool selected, StringView label = U"");

		template<class T, class U = T>
		bool radiobutton(T& target, const U& value, StringView label = U"")
		{
			bool clicked = radiobutton(target == value, label);
			if (clicked)
			{
				target = value;
			}
			return clicked;
		}

		size_t& tab(StringView id, Array<String> tabNames, size_t firstIdx = 0);

		// void dropdown();

		bool simpleColorpicker(HSV& value);

		bool simpleSlider(double& value, double width = 120);

		// template<class T>
		// void spinbox(T& target, T step = static_cast<T>(1), T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max());

		// void split();

		void progressbar(double value, int32 width = 200);

		void custom(std::shared_ptr<IControl> control);

	private:

		std::unique_ptr<detail::GUIImpl> m_impl;

		std::vector<detail::WindowImpl*> m_stack;

		detail::WindowImpl* m_defaultWindow;

		detail::WindowImpl& getDefaultWindowImpl() { return *m_defaultWindow; }

		detail::WindowImpl& getCurrentWindowImpl() { return *m_stack.back(); }

	public:

		~GUIManager();
	};

	class GUIAddon : IAddon
	{
	public:

		GUIManager gui;

	private:

		bool init() override { return true; }

		bool update() override { return true; }

		void draw() const override {}

		void postPresent() override {}
	};
}
