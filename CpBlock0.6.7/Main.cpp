#include "stdafx.h"
# include <Siv3D.hpp> // OpenSiv3D v0.6.7
# include <Siv3D/Windows/Windows.hpp>
#include "SasaGUI/SasaGUI.hpp"

using namespace std;

enum blocktype {
	define,
	print,
	read,
	list_get,
	list_set,
	string_at,
	string_connect,
	int_cal,
	double_cal,
	cast,
	function,
	main,
	system_while,
	system_for,
	system_if,
	system_sleep,
	system_random,
	system_and,
	system_or,
	system_not,
	system_time,
	call_function
};
struct block_data {
	blocktype type = blocktype::print;
};
struct data {
	set<pair<String, vector<String>>> functions;
};

struct scene_data {
	SasaGUI::GUIManager gui;
};

using App = SceneManager<String, scene_data>;



namespace dialog {
	/// @brief ダイアログを表示します。System::Update()の直前に実行してください。
	/// @param message ダイアログに表示するメッセージを入力します。
	/// @param title 必要に応じてタイトルを入力します。
	/// @param size ダイアログのサイズを指定します。
	/// @param background 背景色を指定します。
	inline const void showDialog(String message, Optional<String> title = unspecified, RectF::position_type size = { 400,300 }, Color background = Color{ 128, 128, 128, 224 }) {
		static bool showing = true;
		RectF{ 0,0,Scene::Size().x,Scene::Size().y }.draw(background);
		RectF{ Scene::Center().x - size.x / 2,Scene::Center().y - size.y / 2,size }.draw(Palette::White);
		if (title.has_value()) {
			RectF{ Scene::Center().x - size.x / 2,Scene::Center().y - size.y / 2,size.x,FontAsset(U"font30")(*title).font.fontSize() + 10 }.draw(Palette::Lightgray);
			FontAsset(U"font30")(*title).draw(Vec2{ Scene::Center().x - size.x / 2,Scene::Center().y - size.y / 2 } + Vec2{ 10,0 }, Palette::Black);
		}
		FontAsset(U"font20")(message).draw(Vec2{ Scene::Center().x - size.x / 2,Scene::Center().y - size.y / 2 } + Vec2{ 15,FontAsset(U"font30")(*title).font.fontSize() + 10 },Palette::Black);
	}
};
namespace doubleClick {
	double leftClick = -(double)GetDoubleClickTime()-1, rightClick = -(double)GetDoubleClickTime()-1;
	const bool left() {
		if ((Scene::Time() - leftClick) * 1000 <= GetDoubleClickTime()) {
			return true;
		}
		return false;
	}
	const bool right() {
		if ((Scene::Time() - rightClick) * 1000 <= GetDoubleClickTime()) {
			return true;
		}
		return false;
	}
	const void update() {
		if (MouseL.down()) leftClick = Scene::Time();
		else if (MouseR.down()) rightClick = Scene::Time();
	}
};
namespace fontWidth {
	inline double getWidth(String fontName, String text) {
		return FontAsset(fontName)(text).region().w;
	}
	inline double getWidth(DrawableText text) {
		return text.region().w;
	}
};

Vec2 nullPos{ 0,0 };

template<typename data_type>
class block {
	Vec2 pos;
	Texture texture;
	String fontName = U"font30";
	Image blockImage;
	TextureRegion obj0, obj2;
	RectF objF0, objF2;
	int width;
	RectF obj1;
	Color fontColor = Palette::White;
	Vec2* add_pos;
public:
	block() {}
	block(Vec2 _pos, Texture _block, String _fontName, String _text, Image _blockImage, Vec2* _add_pos = nullptr) :pos{ _pos }, texture{ _block }, fontName{ _fontName }, text{ _text }, blockImage{ _blockImage }, add_pos{ _add_pos } {
		obj0 = texture(0, 0, 86, texture.height() - 1).scaled(0.5);
		obj2 = texture(86, 0, texture.width() - 86, texture.height() - 1).scaled(0.5);
		width = (int)fontWidth::getWidth(fontName, text) / 2;
		obj1 = Rect{ int(pos.x + 43), int(pos.y + 3 / 2) ,width * 2 + 5,texture.height() / 2 - 13/*-3 - 25*/ };
		if (add_pos == nullptr) {
			add_pos = &nullPos;
		}
	}
	void draw() const {
		Color blockColor = blockImage[blockImage.width() / 2][blockImage.height() / 2];
		obj0.draw(pos);
		obj2.draw(Vec2{ pos.x + width * 2 + 85 / 2 + 5,pos.y } + *add_pos);
		obj1.draw(blockColor);
		Vec2 font_pos = { 0,0 };
		bool inGrave = false;
		FontAsset(StringView(fontName))(StringView(text)).drawAt(Vec2{ pos.x + 43 + width + 5, pos.y + 25 } + *add_pos, fontColor);
	}
	void update() {
		width = (int)fontWidth::getWidth(fontName, text) / 2;
		objF0 = obj0.region(pos);
		objF2 = obj2.region(Vec2{ pos.x + width * 2 + 85 / 2,pos.y } + *add_pos);
	}
	void setPos(Vec2 _pos) {
		pos = _pos;
		obj1 = RectF{ Vec2{(pos.x + 43), (pos.y + 3 / 2)} + *add_pos ,width * 2 + 5,texture.height() / 2 - 13/*-3 - 25*/ };
	}
	Vec2 getPos() const {
		return pos;
	}
	int getBlockWidth() noexcept {
		return width * 2 + 107;
	}
	int getFontWidth() noexcept {
		return width * 2;
	}
	void setColor(Color color) {
		fontColor = color;
	}
	String text = U"";
	/// @brief このフレームで押され始めたかを確認します。
	/// @return 押され始めたか
	bool isClicked() const {
		return m_click && objF0.leftClicked() || obj1.leftClicked() || objF2.leftClicked();
	}
	inline const bool isDoubleClicked() const {
		return m_click && isHover() && doubleClick::left();
	}
	bool isReleased() const {
		return m_click && objF0.leftReleased() || obj1.leftReleased() || objF2.leftReleased();
	}
	bool isHover() const {
		return m_click && objF0.mouseOver() || obj1.mouseOver() || objF2.mouseOver();
	}
	data_type data;
	bool m_click = true;
};
template <typename data_type>
class blocks : public vector<block<data_type>> {
public:
	/// @param query: クエリをラムダ式で入力します。
	/// @return クエリに一致するblockを返します。
	vector<block<data_type>&> querySelector(std::function<bool(block<data_type>&)> query) {
		vector<block<data_type>&> re(0);
		for (int i = 0; i < this->size(); i++) {
			if (query(this->at(i))) {
				re.push_back(this->at(i));
			}
		}
		return re;
	}
};

template<typename data_type>
void blockInit(block<data_type>& s) {
	if (s.data.type == blocktype::print) {
		s.data.args.resize(1);
	}
}

class home : public App::Scene {
public:
	home(const InitData& init) :IScene{ init } {
		Platform::Windows::Window::SetTaskbarProgressBar(0.5);
	}
	void draw() const override {
		FontAsset(U"font50")(U"CpBlock - Click to start easy programming").drawAt(Scene::Center());
	}
	void update() override {
		if (System::GetUserActions() & UserAction::MouseButtonDown) {
			changeScene(U"project");
		}
	}
};

class project : public App::Scene {
public:
	struct block_data_type :public block_data {
		bool pressing = false;
		Vec2 has_pos = { 0,0 };
		bool can_connect = true;
		int parent_id = -1, child_id = -1;
		bool can_erase = true;
		vector<String> args;
	};
private:
	//文字列用エスケープ関数
	inline String escape(String value) {
		if (value.starts_with(U'$')) {
			value.insert(value.begin(),U'\\');
		}
	}
	bool building = false;
	//定義： 設置対象のブロックのID。
	int putto = -1;
	int block_erase = -1;
	Texture red_block, flag, beacon, root_block, shadow_texture;
	Image red_block_img, flag_img, root_block_img, shadow_img;
	Texture pin1, pin2, build;
	map<int, String> name;
	bool pressed = false;
	blocks<block_data_type> objs;
	blocks<block_data_type> types;
	//定義： カーソル所持のブロックのID。
	int cursor_has = -1;
	//定義： マウスオーバーのブロックID。
	int cursor_hover = -1;
	//定義： 表示順（a_iはi+1番目に表示されるブロックのID）。
	vector<int> order;
	bool blockBar = true;
	void put_it(int from, int to) {
		int from_last = from;
		set<int> passed1;
		while (objs[from_last].data.child_id != -1) {
			if (passed1.count(from_last))break;
			passed1.insert(from_last);
			from_last = objs[from_last].data.child_id;
		}
		if (objs[to].data.child_id != -1) {
			objs[objs[to].data.child_id].data.parent_id = from_last;
			objs[from_last].data.child_id = objs[to].data.child_id;
		}
		objs[from].data.parent_id = to;
		objs[to].data.child_id = from;
		//DO FOLLOW
		int j = to;
		int m = 0;
		set<int> passed;
		Vec2 position = objs[to].getPos();
		while (j != -1) {
			if (passed.count(j)) break;
			passed.insert(j);
			objs[j].setPos({ position.x,position.y + m * 50 });
			j = objs[j].data.child_id;
			m++;
		}
	}
	block<block_data_type> shadow = block<block_data_type>(Scene::Center(), shadow_texture, U"font30", U"", shadow_img);
	const Array<std::pair<String, Array<String>>> menus{
		{U"ウィンドウ",{U"終了"}},
		{U"表示",{U"🔴ブロック",U"©ライセンス"}}
	};
	SimpleMenuBar menu{ menus };

	bool menu_clicked = false;
	Vec2 menu_pos = { 0,0 };
	bool m_can_move = true;

	//カーソルテクスチャの規定
	
	//ブロック削除
	Texture cursor_delete;

	//ダブルクリックダイアログのID。
	int doubleClickDialog = -1;

	//
	int last_cursor_has = -1;
public:
	project(const InitData& init)
		: IScene{ init },
		red_block{ Image(Resource(U"image/red_block.png")) },
		red_block_img{ Resource(U"image/red_block.png") },
		beacon{ Resource(U"image/block_beacon.png") },
		flag_img{ Resource(U"image/flag.png") },
		flag{ Resource(U"image/flag.png") },
		root_block{ Resource(U"image/root_block.png") },
		root_block_img{ Resource(U"image/root_block.png") },
		shadow_texture{ Resource(U"image/block_shadow.png") },
		shadow_img{ Resource(U"image/block_shadow.png") },
		pin1{ Resource(U"image/pin1.png") },
		pin2{ Resource(U"image/pin2.png") },
		build{ Resource(U"image/build.png") },
		cursor_delete{ Resource(U"cursor/del.png") }
	{
		Scene::SetBackground(Palette::White);
		shadow.setColor(Color(0xc7c7c7));
		menu.setItemChecked(MenuBarItemIndex{ 1,0 }, true);
		objs.push_back(block<block_data_type>(Scene::Center(), flag, U"font30", U"起動したとき", flag_img));
		objs.push_back(block<block_data_type>(Scene::Center(), root_block, U"font30", U"設定", root_block_img));
		objs.push_back(block<block_data_type>(Scene::Center(), red_block, U"font30", U"`REDと出力する", red_block_img));
		objs.push_back(block<block_data_type>(Scene::Center(), red_block, U"font30", U"BLUEと出力する", red_block_img));
		objs[0].data.can_connect = false;
		objs[1].data.can_connect = false;
		objs[0].data.type = blocktype::main;
		objs[1].data.type = blocktype::define;
		objs[0].data.can_erase = false;
		objs[1].data.can_erase = false;
		for (int i = 0; i < objs.size(); i++) {
			blockInit(objs[i]);
			order.push_back(i);
		}
		Platform::Windows::Window::SetTaskbarProgressBar(1);
	}
	void draw() const override {
		if (cursor_has != -1 && putto != -1 && objs[cursor_has].data.can_connect && cursor_has != putto) {
			auto s = objs[putto];
			shadow.draw();
		}
		bool menu_opened = menu.getItemChecked(MenuBarItemIndex{ 1,0 });
		if (menu_opened) {
			Rect{ 0,0,300,Scene::Height() }.draw(Palette::White);
			Line{ 51,0,51,Scene::Height() }.draw(1, Palette::Black);
			Line{ 301,0,301,Scene::Height() }.draw(1, Palette::Black);
		}
		Rect{ 0,Scene::Height() - 50,50,50 }.drawShadow({ 0,0 }, 5, 1.5, Palette::Black);

		if (menu_opened) {
			Rect{ 0,Scene::Height() - 102,50,50 }.drawShadow({ 0,0 }, 5, 1.5, Palette::Black).draw(Color(0x4c, 0x97, 0xff));
			Rect{ 0,Scene::Height() - 50,50,50 }.draw(Color(0x4c, 0x97, 0xff));
			build.resized(32).drawAt(25, Scene::Height() - 76);

			pin1.resized(32).drawAt(25, Scene::Height() - 25);
		}
		else {
			Rect{ 0,Scene::Height() - 50,50,50 }.draw(Color(0x4c, 0x97, 0xff));
			pin2.resized(32).drawAt(25, Scene::Height() - 25);
		}
		for (int i = objs.size() - 1; i != -1; i--) {
			objs.at(order[i]).draw();
		}
		if (building) {
			dialog::showDialog(U"ビルド中です。しばらくお待ち下さい。", U"ビルド");
		}
		menu.draw();
		if (cursor_has != -1 && Rect{ 0,0,300,Scene::Height() }.mouseOver() && menu_opened && objs[cursor_has].data.can_connect) {
			cursor_delete.resized(32).draw(Cursor::PosF());
		}
	}
	void update() override {
		if (menu_clicked && Rect{ 0,0,300,Scene::Height() }.mouseOver()) {
			menu_pos.y += Mouse::Wheel();
			if (menu_pos.y < 0) menu_pos.y = 0;
		}
		bool menu_opened = menu.getItemChecked(MenuBarItemIndex{ 1,0 });
		if (!menu_opened) {
			menu_clicked = false;
		}
		//ブロックの削除処理

		Print << cursor_has << (block_erase < objs.size()) << menu_opened << MouseL.up();
		if (block_erase != -1 && block_erase < objs.size() && menu_opened && MouseL.up()) {
			if (objs[block_erase].data.can_erase) {
				int i = block_erase;
				while (i != -1) {
					int next = objs[i].data.child_id;
					objs.erase(objs.begin() + i);
					order.erase(find(order.begin(), order.end(), i));
					i = next;
				}
			}
			else {
				int i = block_erase,j = 0;
				while (i != -1) {
					objs[i].setPos(Scene::Center() + Vec2{ 0,j * 50 });
					i = objs[i].data.child_id;
					j++;
				}
			}
		}

		if (cursor_has != -1 && Rect{ 0,0,300,Scene::Height() }.mouseOver()) {
			block_erase = cursor_has;
		}
		if (Rect{ 0,Scene::Height() - 102,50,50 }.leftClicked()) {
			m_can_move = false;
			building = true;
		}
		else if (!Rect{ 0,0,300,Scene::Height() }.mouseOver() && !MouseL.pressed()) {
			block_erase = -1;
		}
		if (Rect{ 0,Scene::Height() - 50,50,50 }.leftClicked()) {
			menu.setItemChecked(MenuBarItemIndex{ 1,0 }, !menu.getItemChecked(MenuBarItemIndex{ 1,0 }));
			if (menu.getItemChecked(MenuBarItemIndex{ 1,0 })) {
				menu_clicked = true;
			}
		}
		if (Rect{ 0, 0, 300, Scene::Height() }.leftClicked()) {
			menu_clicked = true;
		}
		if ((MouseL.down() || MouseR.down()) && !Rect { 0, 0, 300, Scene::Height() }.mouseOver()) {
			menu_clicked = false;
		}
		putto = -1;
		if (const auto item = menu.update()) {
			if (item == MenuBarItemIndex{ 0,0 }) {
				System::Exit();
			}
			if (item == MenuBarItemIndex{ 1,0 }) {
				blockBar = !blockBar;
				menu.setItemChecked(*item, !menu.getItemChecked(*item));
			}
			if (item == MenuBarItemIndex{ 1,1 }) {
				LicenseManager::ShowInBrowser();
			}
		}
		for (int i = 0; i < objs.size(); i++) {
			if (objs[i].isHover())
				Cursor::RequestStyle(CursorStyle::Hand);
			Vec2 cursor = Cursor::Pos(), obj = objs[order[i]].getPos();
			double x = cursor.x - obj.x, y = objs[cursor_has].getPos().y - obj.y;
			if (y <= 55 && y >= 45 && x >= 0 && x <= objs[order[i]].getBlockWidth()) {
				putto = order[i];
				break;
			}
		}
		if (cursor_has != -1 && putto != -1 && objs[cursor_has].data.can_connect && cursor_has != putto) {
			shadow.setPos(objs[putto].getPos() + Vec2{ 0,50 });
		}
		if (MouseL.up() && putto != -1 && cursor_has != -1 && putto != cursor_has && objs[cursor_has].data.can_connect) {
			put_it(cursor_has, putto);
		}
		for (int i = 0; i < objs.size(); i++) {
			objs[i].update();
			objs[order[i]].update();
			//最も上のブロックのClick時処理
			if (objs[order[i]].isClicked() && cursor_has == -1) {
				objs[order[i]].data.pressing = true;
				objs[order[i]].data.has_pos = objs[order[i]].getPos() - Cursor::Pos();
				if (m_can_move) {
					cursor_has = order[i];
				}
			}
			else if (!MouseL.pressed()) {
				cursor_has = -1;
				objs[order[i]].data.pressing = false;
			}
			//最も上のブロックのHover時処理
			if (objs[order[i]].isHover() && cursor_hover == -1) {
				cursor_hover = i;
			}
			//最も上のブロックのDoubleClick時処理
			if (objs[order[i]].isDoubleClicked() && doubleClickDialog == -1) {
				doubleClickDialog = order[i];
				Print << doubleClickDialog;
			}
			if (objs[i].data.pressing && m_can_move) {
				//DISCONNECT
				if (objs[i].data.parent_id != -1)
					objs[objs[i].data.parent_id].data.child_id = -1;
				objs[i].data.parent_id = -1;
				//DO FOLLOW
				int j = i;
				int m = 0;
				set<int> passed;
				while (j != -1) {
					if (passed.count(j)) break;
					passed.insert(j);
					Vec2 position = (Cursor::Pos() + objs[i].data.has_pos);
					objs[j].setPos({ position.x,position.y + m * 50 });
					j = objs[j].data.child_id;
					m++;
				}
			}
		}
		for (int i = 0; i < objs.size(); i++) {
			if (objs[order[i]].data.pressing) {
				auto c = order[i];
				order.erase(order.begin() + i);
				order.insert(order.begin(), c);
			}
		}
		if (cursor_has != -1) {
			Cursor::RequestStyle(CursorStyle::ResizeAll);
		}
		if (cursor_has != -1 && putto != -1 && objs[cursor_has].data.can_connect && cursor_has != putto) {
			Cursor::RequestStyle(CursorStyle::Arrow);
		}

		//ブロック削除時のカーソルの変更
		{
			if (cursor_has != -1 && Rect{ 0,0,300,Scene::Height() }.mouseOver() && menu_opened) {
				if (objs[cursor_has].data.can_erase) {
					Cursor::RequestStyle(CursorStyle::Hidden);
				}
				else {
					Cursor::RequestStyle(CursorStyle::NotAllowed);
				}
			}
		}
		if (cursor_has != -1) last_cursor_has = cursor_has;
		auto& gui = getData().gui;
		gui.windowBegin(U"Debug", SasaGUI::WindowFlag::Debug | SasaGUI::WindowFlag::AutoResize);
		gui.label(U"ID:{}"_fmt(last_cursor_has));
		if (last_cursor_has != -1) {
			gui.label(U"Parent:{}"_fmt(objs[last_cursor_has].data.parent_id));
			gui.label(U"Child:{}"_fmt(objs[last_cursor_has].data.child_id));
			gui.label(U"Pos:{}"_fmt(objs[last_cursor_has].getPos()));
		}
		gui.windowEnd();
	}
};

void Main()
{
	auto shared = make_shared<scene_data>();
	Scene::SetBackground(Color(12, 222, 222));
	Window::SetStyle(WindowStyle::Sizable);
	Window::Maximize();
	App app(shared);
	FontAsset::Register(U"font20", FontMethod::MSDF, 20);
	FontAsset::Register(U"font30", FontMethod::MSDF, 30);
	FontAsset::Register(U"font50", FontMethod::MSDF, 50);
	LicenseInfo info;
	info.title = U"CpBlock";
	info.copyright = U"Copyright (c) 2022-2023 Yama_can";
	info.text = Unicode::FromUTF8(R"(Permission to download and edit the source code of this application is granted free of charge.
In addition, redistribution and commercial use of the source code shall comply with the following conditions.
Please state the license including Siv3D Engine, CpBlock, put the license display button in a conspicuous place, and be sure to consult the creator when using it for commercial purposes.

We are not responsible if this app violates memory security and destroys or damages your computer or any other loss.

contact address
Mail: yama.can95@gmail.com
Twitter: @w_yama_can)");
	LicenseManager::AddLicense(info);
	app.add<home>(U"start");
	app.add<project>(U"project");
	auto& gui = app.get()->gui;
	 do{
		doubleClick::update();
		gui.frameBegin();
		if (!app.update()) {
			System::Exit();
		}
		gui.frameEnd();
	 } while (System::Update());
}

