#include "scene.h"
#include "CSVResource.h"
#include "progress.h"
using namespace MG;

namespace DemoScene {
	// =======================================================
	// クラス定義
	// =======================================================
	class DemoScene : public Scene {
	public:
		void Init() override;
		//void Uninit() override;
		//void Update() override;
		//void Draw() override;
		//LAYER_TYPE GetLayerType(int layer) override;
	};


	// =======================================================
	// シーン登録
	// 登録すれば「SceneTransit(シーン名、遷移名)」で呼び出せる
	// 実は関数ではなく定型文マクロ
	// 「static SceneName xxx = 」の部分は必須
	// 
	// 引数１	シーン名
	// 引数２	シーンのクラス名
	// =======================================================
	static SceneName sceneName = REGISTER_SCENE("empty", DemoScene);

	// =======================================================
	// 初期化
	// シーンロードされた時に呼び出される
	// 
	// ※一度呼び出されたシーンのインスタンスは
	// シーンが終わった後も保管されるので、
	// Init()はシーン遷移するたびに呼び出されるに対して
	// コンストラクタは初回だけ。
	// 用途によって使い分けてください。
	// =======================================================
	void DemoScene::Init()
	{
		Scene::Init();
		
		// =======================================================
		// GameObjectの使用方法
		// 
		// GameObject自体はただの基底クラスなので
		// Update()もDraw()も空
		// 実際は継承して使ってください
		// =======================================================
		if (false) {
			// GameObjectをシーンに追加する
			// シーンがUpdate()とDraw()する時、
			// GameObjectのUpdate()とDraw()も勝手に回される
			AddGameObject(GameObject());

			// こっちは順番を指定してシーンに追加する
			// この順番はUpdate()とDraw()の順番になる
			// ただしDraw()時はlayer順が優先される
			InsertGameObject(GameObject(), 2);

			// レイヤー指定して追加
			// デフォルトは0～9までは3Dそれ以外は2D
			// どのレイヤーが2Dか3Dかは自分で定義できる、
			// GetLayerType(int layer)を参照してください
			const int ANY_LAYER_NUMBER = 1;
			AddGameObject(GameObject(), ANY_LAYER_NUMBER);
			InsertGameObject(GameObject(), 2, ANY_LAYER_NUMBER);
			AddGameObject(GameObject(), LAYER_2D); // 手前のUIに使うとか
			AddGameObject(GameObject(), -1); // 逆に奥の2Dオブジェクトを貼りたい時

			// 追加したもののポインタは返される
			GameObject* ptr0 = AddGameObject(GameObject());
			GameObject* ptr1 = InsertGameObject(GameObject(), 3);

			ptr0->position = { 0.0f, 0.0f, 0.0f }; // x, y, z
			ptr0->size = { 1.0f, 1.0f, 1.0f }; // 正確はサイズの倍率、同じくx, y, z
			ptr0->rotate = Quaternion::Identity(); // これは無回転

			// F2, F3, F4の計算 ※一部実装してない
			ptr0->position += { 10.0f, -20.0f, 0.0f}; // 加算したり
			ptr0->size *= 2.0f; // 倍にしたり
			ptr0->size *= F3{ 1.0f, 2.0f, 3.0f }; // 軸を一つずつ指定
			
			// クォータニオンの作成関数たち
			Quaternion::AxisRadian({ 0.5f, 0.5f, 0.0f }, 0.1f);
			Quaternion::AxisXRadian(0.1f);
			Quaternion::AxisYRadian(0.1f);
			Quaternion::AxisZRadian(0.1f);
			Quaternion::AxisDegree({ 0.5f, 0.5f, 0.0f }, 1.0f);
			Quaternion::AxisXDegree(1.0f);
			Quaternion::AxisYDegree(1.0f);
			Quaternion::AxisZDegree(1.0f);

			// 掛け算にすれば、「元の回転状態からさらに回転する」っていう意味になる
			ptr0->rotate *= Quaternion::AxisZRadian(0.5f);

			// GameObjectのenableをfalseにすればUpdate()とDraw()時はスキップされる
			ptr0->enable = false;

			// GameObjectをシーンから削除
			// シーンと違って即座インスタンス消すので注意
			// なお、AddGameObjectで追加したGameObjectは
			// シーン終了（別のシーンに移行したとか）時に
			// 勝手に消去されるので、
			// 後始末の意味で追一DeleteGameObject()する必要はナシ
			DeleteGameObject(ptr0);
			ptr0 = nullptr;

		}


		// =======================================================
		// リソース
		// 
		// 同じパスのファイルのリソースは重複してロードしないが、
		// リソースプールから走査する手間がかかるので
		// Update()やDraw()の中に毎フレーム呼び出すのは避けた方が吉
		// 
		// このシーンでLoadしたリソースは、
		// このシーンが終了時に勝手に削除される
		// ただし別のシーンで使用中のリソースは除く
		// =======================================================
		if (false) {

			// =======================================================
			// リソースロード
			// =======================================================
			const char* TEXTURE_FILE = "asset\crusade\white.png";
			const char* AUDIO_FILE = "asset\crusade\bgm.wav";
			const char* MODEL_FILE = "asset\crusade\model.mgo";
			Texture* texture = LoadTexture(TEXTURE_FILE);
			Audio* audio = LoadAudio(AUDIO_FILE);
			Model* model = LoadModel(MODEL_FILE);

			// スコープを指定してロード
			// シーンの自動後始末から外される
			// 何か重くて、各シーン共通して使うリソースは
			// この方法で前もってロードするとかに使う
			LoadTexture(TEXTURE_FILE, "anyScope");
			LoadAudio(AUDIO_FILE, "anyScope");
			LoadModel(MODEL_FILE, "anyScope");

			// スコープ指定しない場合は、
			// このシーンのsceneNameをスコープとして使用するので
			// 以下の二つは同じ意味になる
			LoadTexture(TEXTURE_FILE);
			LoadTexture(TEXTURE_FILE, sceneName);


			// =======================================================
			// リソース解放
			// 
			// 後始末はシーン終了時自動でするので
			// 独自のスコープ指定してる場合か、
			// なにかデカいリソースをシーン中に解放したい場合以外は使わなくていい
			// =======================================================
			ReleaseTexture(sceneName);	// このシーンの全テクスチャ
			ReleaseAudio(sceneName);	// このシーンの全音声
			ReleaseModel(sceneName);	// このシーンの全モデル
			ReleaseResource(sceneName);	// このシーンの全リソース（種類問わず）
			ReleaseTexture(TEXTURE_FILE, sceneName);	// このシーンの特定のテクスチャ
			ReleaseTexture(TEXTURE_FILE, "anyScope");	// "anyScope"というスコープの特定のテクスチャ


			// =======================================================
			// 各リソースの使用例
			// =======================================================

			// 板状のGameObject
			AddGameObject(
				GameObjectQuad(
					LoadTexture(TEXTURE_FILE),
					{ 1.0f, 1.0f, 1.0f },		// size
					{ 0.0f, 0.0f, 0.0f }		// position
				)
			);

			// GameObject介さず直接にDraw
			// 
			// uvの最小値 = uvOffset
			// uvの最大値 = uvOffset + uvRange
			DrawQuad(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size, 2Dの場合はピクセル
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }, // color
				{ 0.0f, 0.0f },				// uvOffset
				{ 1.0f, 1.0f }				// uvRange
			);

			// 最小限の引数
			DrawQuad(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f }		// size
			);

			// CubeをDraw
			// 
			// テクスチャのuv値は天井、胴体（四面共通）、底の順で一列に並ぶ
			// 独自のuv値を使いたい場合はDrawModel()を使ってください
			DrawCube(
				LoadTexture(TEXTURE_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }  // color
			);


			// モデルをDraw
			DrawModel(
				LoadModel(MODEL_FILE),
				{ 0.0f, 0.0f, 0.0f },		// position
				{ 1.0f, 1.0f, 1.0f },		// size
				Quaternion::Identity(),		// rotate
				{ 1.0f, 1.0f, 1.0f, 1.0f }  // color
			);

			// ModelのMeshごとのテクスチャは手動で変えることも可能
			// ただし、モデルとテクスチャの解放のタイミングが食い違う場合もあるので注意
			model->meshTextures[model->rawModel->meshes] = LoadTexture(TEXTURE_FILE);

			// 音声はプレイヤーを準備する必要あるので
			// GameObjectAudioをシーンに追加することをおススメ
			GameObjectAudio* audioObject = AddGameObject(GameObjectAudio(audio, true));
			audioObject->Play();
			audioObject->SetVolume(0.5f);
			audioObject->SetLoop(false);
			audioObject->Stop();
		}
		
		// =======================================================
		// シーンの移行
		// 
		// シーン遷移中、二つ以上のシーンが同時に稼働してることもある
		// シーンがスタート・エンドされるタイミングや
		// どのシーンがDrawされるかは
		// 遷移プログラムに依存してるので注意
		// =======================================================
		if (false) {

			// 引数１　移行先のシーン名
			// 引数２　遷移名（エフェクトなど）
			SceneTransit("test", "sample_transition");

			// 遷移名記載しない場合はぱっと切り替える
			SceneTransit("test");

			// 現在はシーン遷移の途中かどうかをチェック
			// 「シーン遷移が完了するまでは動かない」とかに使う
			bool inTransition = InTransition();
		}


		// =======================================================
		// ファイル間のデータ交換
		// 
		// 同じ名前を入れれば別のファイルでもセットした値を取得することができる
		// シーンだけじゃなくて基本どこでも使えるので
		// 場合によってシーン遷移やオブジェクトの設定に使用することも可能
		// =======================================================
		if (false) {
			SetCommonBool("any_bool", true);
			bool anyBool = GetCommonBool("any_bool");

			SetCommonFloat("any_float", 0.5f);
			bool anyFloat = GetCommonFloat("any_float");

			SetCommonInt("any_int", 30);
			int anyInt = GetCommonInt("any_int");

			SetCommonString("any_string", "任意の文字列");
			std::string anyString = GetCommonString("any_string");

			// 任意のポインタ入れることもできるがちょっと危険
			GameObject* ptr0 = AddGameObject(GameObject());
			SetCommonPointer("any_pointer", ptr0);
			GameObject* anyPointer = (GameObject*)GetCommonPointer("any_pointer");
		}


		// =======================================================
		// CSVファイルの読み込み
		// 
		// #include "CSVResource.h"を加える必要あり
		// =======================================================
		if (false) {
			D_TABLE table;
			D_KVTABLE keyValuePair;
			const char* CSV_FILE = "asset\\config.csv";
			ReadCSVFromPath(CSV_FILE, table);

			int rowNum = table.size(); // 行の数
			int colNum = table[0].size(); // 列の数
			std::string row0Col1 = table[0][1]; // 0行1列の内容（文字列）

			// TableデータからKey-Value-Pairへ変換
			// 引数１　キーの文字列
			// 
			// 一行目を列の名前に見立てて
			// さらにキーの文字列が載ってる列をキー列として
			// keyValuePair[キー][列名]という構造に変換する
			TableToKeyValuePair("key", table, keyValuePair);

			// 例えば以下のcsvデータがあるとして
			// key,				value
			// TEXTURE_WHITE,	asset\crusade\white.png
			// TEXTURE_RED,		asset\crusade\red.png
			// TEXTURE_GREEN,	asset\crusade\green.png
			// ENEMY_NUM,		100
			// PLAYER_SPEED,	50.0

			std::string textureWhitePath = keyValuePair["TEXTURE_WHITE"]["value"]; //　asset\crusade\white.png
			std::string textureRedPath = keyValuePair["TEXTURE_RED"]["value"]; //　asset\crusade\red.png
			std::string textureGreenPath = keyValuePair["TEXTURE_GREEN"]["value"]; //　asset\crusade\green.png
			int enemyNum = std::stof(keyValuePair["ENEMY_NUM"]["value"]); // 100
			float playerSpeed = std::stof(keyValuePair["PLAYER_SPEED"]["value"]); // 50.0f
		}
		
		// =======================================================
		// プログレス
		// 
		// 数学で言うtの代わりになる便利クラス
		// #include "progress.h"を加える必要あり
		// =======================================================
		if (false) {
			// 引数１　最大絶対値
			// 引数２　ループするか
			Progress t(1000.0f, false);

			int deltaTime = GetDeltaTime(); // 前フレームからの経過時間
			t.IncreaseValue(deltaTime);		// tに絶対値を累積する

			// この例だと、最大絶対値は1000.0fで
			// 仮にGetDeltaTime()で取得した値は17として（1000ミニ秒÷60フレーム）
			// 現在のtは「17.0f / 1000.0f」同等になる
			// つまり0.017
			// 値は0.0f～1.0fの範囲からはみ出すことはない（足切り、頭打ち）
			F3 startPosition = { 10.0f, 20.0f, 30.0f }; // 出発点
			F3 endPosition = { 30.0f, 0.0f, 10.0f };	// 目的地

			// 現在地
			F3 currentPosition = startPosition * (1.0f - t) + endPosition * t;

			// 他にも色の変色やフェイドインアウトや
			// クールタイムの判定などに使える
			Progress coolTime(5000.0f, false);
			if (coolTime == 1.0f) {
				coolTime.SetValue(0.0f); // 絶対値を0.0fに戻す
			}
			coolTime.IncreaseValue(GetDeltaTime());
			coolTime.IncreaseValue(GetDeltaTime() * 2.0f); // 倍速にしたり

			// なにかのカウンターに使うとか
			Progress counter(10.0f, false);
			if (counter == 1.0f) {
				// 処理
			}
			counter.IncreaseValue(1);

			// ループ設定
			// 絶対値が頭打ちになったら自動で0.0fに戻る
			Progress alpha(500.0f, true); // 第二引数trueでオン
			F4 color = { 1.0f, 0.0f, 0.0f, alpha }; // 500ミニ秒の周期で点滅する
			alpha.IncreaseValue(GetDeltaTime());
		}


		
	}


	// =======================================================
	// 終了処理
	// シーンエンドの時に呼び出される
	// 
	// ※Init()と同様、Uninit()シーン遷移するたびに呼び出されるが
	// インスタンス自体は保管されるので、
	// デストラクタはこの時点では実行しない。
	// ReleaseScene()で強制的にインスタンス消すことも可能だが、
	// 特に理由はない限りそうする必要ないし、おススメしない。
	// =======================================================
	/*void DemoScene::Uninit()
	{
		Scene::Uninit();
	}*/


	// =======================================================
	// 更新
	// 基底クラスのScene::Update()を呼び出すと
	// AddGameObject()で追加した全GameObjecctのUpdate()を実行する
	// ただしenableメンバがfasleになってるGameObjecctは除く
	// =======================================================
	/*void DemoScene::Update()
	{
		Scene::Update();
	}*/


	// =======================================================
	// 描画
	// Update()と同じく
	// =======================================================
	/*void DemoScene::Draw()
	{
		Scene::Draw();
	}*/


	// =======================================================
	// レイヤータイプの定義
	// 描画する時、GameObjectに設定したlayerに応じて
	// 2Dのカメラと3Dのカメラどっちを適用するのかを分ける。
	// この関数を継承して「３番なら2Dで、４番は3D」みたいに、
	// カスタマイズできる。
	// デフォルトは0～９までは3Dそれ以外は2D
	// 
	// ※layerは2Dと3Dを分ける以外に描画順でもあり、
	// あるいは「特定のlayerのオブジェクトだけ～」
	// っていう使い方したりできる
	// =======================================================
	/*LAYER_TYPE DemoScene::GetLayerType(int layer)
	{
		return Scene::GetLayerType(layer);
	}*/
}
