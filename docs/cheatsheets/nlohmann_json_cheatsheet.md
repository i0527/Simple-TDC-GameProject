# nlohmann/json 関数リファレンス チートシート

nlohmann/json (JSON for Modern C++) の主要なAPIをまとめたクイックリファレンスです。

## 目次

- [基本設定](#基本設定)
- [JSONオブジェクトの作成](#jsonオブジェクトの作成)
- [パース（文字列からJSON）](#パース文字列からjson)
- [シリアライゼーション（JSONから文字列）](#シリアライゼーションjsonから文字列)
- [アクセス](#アクセス)
  - [オブジェクトアクセス](#オブジェクトアクセス)
  - [配列アクセス](#配列アクセス)
  - [安全なアクセス](#安全なアクセス)
- [型チェック](#型チェック)
- [型変換](#型変換)
- [STLライクな操作](#stlライクな操作)
  - [イテレータ](#イテレータ)
  - [コンテナ操作](#コンテナ操作)
- [JSON Pointer](#json-pointer)
- [JSON Patch / Merge Patch](#json-patch--merge-patch)
- [ストリーム入出力](#ストリーム入出力)
- [カスタムシリアライゼーション](#カスタムシリアライゼーション)
- [バイナリ形式](#バイナリ形式)
- [エラーハンドリング](#エラーハンドリング)
- [よく使うパターン](#よく使うパターン)

---

## 基本設定

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// または
#include <nlohmann/json.hpp>
using namespace nlohmann;
```

---

## JSONオブジェクトの作成

```cpp
// 空のJSONオブジェクト（null）
json j;

// 初期化リストから作成
json j = {
    {"pi", 3.141},
    {"happy", true},
    {"name", "Niels"},
    {"nothing", nullptr},
    {"answer", {
        {"everything", 42}
    }},
    {"list", {1, 0, 2}},
    {"object", {
        {"currency", "USD"},
        {"value", 42.99}
    }}
};

// 段階的に構築
json j;
j["pi"] = 3.141;
j["happy"] = true;
j["name"] = "Niels";
j["nothing"] = nullptr;
j["answer"]["everything"] = 42;
j["list"] = {1, 0, 2};
j["object"] = {{"currency", "USD"}, {"value", 42.99}};

// 空の配列・オブジェクト
json empty_array = json::array();
json empty_object = json::object();

// 配列の明示的な作成
json arr = json::array({1, 2, 3});
json arr2 = json::array({{"key", "value"}});  // 配列の配列

// オブジェクトの明示的な作成
json obj = json::object({{"key", "value"}});
```

---

## パース（文字列からJSON）

```cpp
// 文字列リテラルからパース
using namespace nlohmann::literals;
json j = R"({"happy": true, "pi": 3.141})"_json;

// json::parse()を使用
json j = json::parse(R"({"happy": true, "pi": 3.141})");
json j = json::parse("{\"happy\": true, \"pi\": 3.141}");

// std::stringからパース
std::string str = R"({"happy": true, "pi": 3.141})";
json j = json::parse(str);

// イテレータ範囲からパース
std::vector<std::uint8_t> v = {'t', 'r', 'u', 'e'};
json j = json::parse(v.begin(), v.end());
json j = json::parse(v);  // コンテナから直接

// エラーハンドリング付きパース
try {
    json j = json::parse(str);
} catch (json::parse_error& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}

// パースエラーを無視
json j = json::parse(str, nullptr, false);
if (j.is_discarded()) {
    // パース失敗
}
```

---

## シリアライゼーション（JSONから文字列）

```cpp
json j = {{"happy", true}, {"pi", 3.141}};

// コンパクト形式
std::string compact = j.dump();
// {"happy":true,"pi":3.141}

// インデント付き（pretty print）
std::string pretty = j.dump(4);
// {
//     "happy": true,
//     "pi": 3.141
// }

// カスタムインデント文字
std::string custom = j.dump(2, '\t');  // タブでインデント

// エラーハンドラー指定
std::string result = j.dump(-1, ' ', false, 
    json::error_handler_t::replace);
```

---

## アクセス

### オブジェクトアクセス

```cpp
json j = {{"name", "John"}, {"age", 30}};

// 演算子[]によるアクセス（存在しない場合は作成）
j["name"] = "Jane";
std::string name = j["name"];

// at()によるアクセス（存在しない場合は例外）
std::string name = j.at("name");
int age = j.at("age");

// value()によるアクセス（デフォルト値付き）
std::string name = j.value("name", "Unknown");
int age = j.value("age", 0);

// ネストしたアクセス
j["person"]["name"] = "John";
std::string name = j["person"]["name"];

// 存在確認
bool has_name = j.contains("name");
bool has_age = j.contains("age");

// 検索
auto it = j.find("name");
if (it != j.end()) {
    std::cout << it.value() << std::endl;
}

// カウント
size_t count = j.count("name");  // 0 or 1
```

### 配列アクセス

```cpp
json j = {1, 2, 3, 4, 5};

// インデックスアクセス
int first = j[0];
int second = j[1];

// at()によるアクセス（範囲チェック付き）
int first = j.at(0);
int last = j.at(j.size() - 1);

// 範囲外アクセスは例外
try {
    int value = j.at(100);
} catch (json::out_of_range& e) {
    std::cerr << "Out of range: " << e.what() << std::endl;
}

// 先頭・末尾要素
json first = j.front();
json last = j.back();
```

### 安全なアクセス

```cpp
json j = {{"name", "John"}, {"age", 30}};

// get_ptr()でポインタ取得（存在しない場合はnullptr）
auto* name_ptr = j.get_ptr<std::string*>("name");
if (name_ptr) {
    std::cout << *name_ptr << std::endl;
}

// value()でデフォルト値付きアクセス
std::string name = j.value("name", "Unknown");
int age = j.value("age", 0);

// JSON Pointerを使用した安全なアクセス
json j = {{"person", {{"name", "John"}, {"age", 30}}}};
std::string name = j.value("/person/name"_json_pointer, "Unknown");
```

---

## 型チェック

```cpp
json j;

// 基本型チェック
j.is_null();
j.is_boolean();
j.is_number();
j.is_number_integer();
j.is_number_unsigned();
j.is_number_float();
j.is_object();
j.is_array();
j.is_string();
j.is_binary();
j.is_discarded();

// プリミティブ型チェック
j.is_primitive();  // null, string, boolean, number, binary

// 構造化型チェック
j.is_structured();  // object, array

// 型の取得
json::value_t type = j.type();
```

---

## 型変換

```cpp
json j = "Hello";
json j_num = 42;
json j_bool = true;

// get()による型変換
std::string str = j.get<std::string>();
int num = j_num.get<int>();
bool b = j_bool.get<bool>();

// get_to()による型変換（既存変数に代入）
std::string str;
j.get_to(str);

int num;
j_num.get_to(num);

// テンプレートget()（推奨）
auto str = j.template get<std::string>();
auto num = j_num.template get<int>();

// 配列からstd::vectorへ
json j = {1, 2, 3, 4};
std::vector<int> vec = j.get<std::vector<int>>();

// オブジェクトからstd::mapへ
json j = {{"one", 1}, {"two", 2}};
std::map<std::string, int> map = j.get<std::map<std::string, int>>();

// 型変換の失敗時は例外
try {
    int num = j.get<int>();  // jが文字列の場合
} catch (json::type_error& e) {
    std::cerr << "Type error: " << e.what() << std::endl;
}
```

---

## STLライクな操作

### イテレータ

```cpp
json j = {{"one", 1}, {"two", 2}, {"three", 3}};

// 前方イテレータ
for (auto it = j.begin(); it != j.end(); ++it) {
    std::cout << it.key() << " : " << it.value() << std::endl;
}

// 範囲ベースforループ
for (auto& element : j) {
    std::cout << element << std::endl;
}

// items()を使用（キーと値の両方）
for (auto& el : j.items()) {
    std::cout << el.key() << " : " << el.value() << std::endl;
}

// 構造化バインディング（C++17）
for (auto& [key, value] : j.items()) {
    std::cout << key << " : " << value << std::endl;
}

// 逆イテレータ
for (auto it = j.rbegin(); it != j.rend(); ++it) {
    std::cout << *it << std::endl;
}

// 配列のイテレーション
json arr = {1, 2, 3, 4, 5};
for (auto& element : arr) {
    std::cout << element << std::endl;
}
```

### コンテナ操作

```cpp
json j;

// サイズ情報
j.size();
j.empty();
j.max_size();

// 配列操作
json arr = json::array();
arr.push_back(1);
arr.push_back(2);
arr.push_back(3);
arr.emplace_back(4);  // インプレース構築

// 挿入
arr.insert(arr.begin() + 1, 10);
arr.insert(arr.end(), {5, 6, 7});

// 削除
arr.erase(arr.begin());  // イテレータで削除
arr.erase(0);  // インデックスで削除
arr.erase(arr.begin(), arr.begin() + 2);  // 範囲削除

// オブジェクト操作
json obj;
obj["key1"] = "value1";
obj.emplace("key2", "value2");  // インプレース構築

// キーの削除
obj.erase("key1");
size_t erased = obj.erase("key2");  // 削除された要素数（0 or 1）

// クリア
j.clear();

// 先頭・末尾要素
json first = j.front();
json last = j.back();
```

---

## JSON Pointer

```cpp
using namespace nlohmann::literals;

json j = {
    {"baz", {"one", "two", "three"}},
    {"foo", "bar"}
};

// JSON Pointerによるアクセス
json value = j["/baz/1"_json_pointer];  // "two"

// JSON Pointerオブジェクトの作成
json_pointer ptr("/baz/1");
json value = j[ptr];

// 存在確認
bool exists = j.contains("/baz/1"_json_pointer);

// value()と組み合わせ
std::string val = j.value("/baz/1"_json_pointer, "default");
```

---

## JSON Patch / Merge Patch

### JSON Patch

```cpp
using namespace nlohmann::literals;

// 元のJSON
json original = R"({
  "baz": ["one", "two", "three"],
  "foo": "bar"
})"_json;

// パッチの適用
json patch = R"([
  { "op": "replace", "path": "/baz", "value": "boo" },
  { "op": "add", "path": "/hello", "value": ["world"] },
  { "op": "remove", "path": "/foo"}
])"_json;

json result = original.patch(patch);
// {
//   "baz": "boo",
//   "hello": ["world"]
// }

// 2つのJSON間の差分を計算
json diff = json::diff(result, original);
// [
//   { "op": "replace", "path": "/baz", "value": ["one", "two", "three"] },
//   { "op": "remove", "path": "/hello" },
//   { "op": "add", "path": "/foo", "value": "bar" }
// ]
```

### JSON Merge Patch

```cpp
using namespace nlohmann::literals;

// 元のJSON
json document = R"({
  "a": "b",
  "c": {
    "d": "e",
    "f": "g"
  }
})"_json;

// マージパッチ
json patch = R"({
  "a": "z",
  "c": {
    "f": null
  }
})"_json;

// パッチの適用
document.merge_patch(patch);
// {
//   "a": "z",
//   "c": {
//     "d": "e"
//   }
// }
```

---

## ストリーム入出力

```cpp
// ファイルからの読み込み
std::ifstream ifs("data.json");
json j;
ifs >> j;

// ファイルへの書き込み
std::ofstream ofs("output.json");
ofs << std::setw(4) << j << std::endl;  // pretty print

// 標準入出力
std::cin >> j;
std::cout << j << std::endl;
std::cout << std::setw(2) << j << std::endl;

// 文字列ストリーム
std::stringstream ss;
ss << j;
json j2;
ss >> j2;
```

---

## カスタムシリアライゼーション

```cpp
// カスタム型のシリアライゼーション
struct Person {
    std::string name;
    int age;
};

// to_json()の実装
void to_json(json& j, const Person& p) {
    j = json{{"name", p.name}, {"age", p.age}};
}

// from_json()の実装
void from_json(const json& j, Person& p) {
    j.at("name").get_to(p.name);
    j.at("age").get_to(p.age);
}

// 使用例
Person person{"John", 30};
json j = person;  // to_json()が呼ばれる

Person person2;
j.get_to(person2);  // from_json()が呼ばれる
```

---

## バイナリ形式

```cpp
// BSON
std::vector<std::uint8_t> bson = json::to_bson(j);
json j2 = json::from_bson(bson);

// CBOR
std::vector<std::uint8_t> cbor = json::to_cbor(j);
json j2 = json::from_cbor(cbor);

// MessagePack
std::vector<std::uint8_t> msgpack = json::to_msgpack(j);
json j2 = json::from_msgpack(msgpack);

// UBJSON
std::vector<std::uint8_t> ubjson = json::to_ubjson(j);
json j2 = json::from_ubjson(ubjson);

// BJData
std::vector<std::uint8_t> bjdata = json::to_bjdata(j);
json j2 = json::from_bjdata(bjdata);
```

---

## エラーハンドリング

```cpp
// パースエラー
try {
    json j = json::parse(invalid_json);
} catch (json::parse_error& e) {
    std::cerr << "Parse error at position " << e.byte << ": " 
              << e.what() << std::endl;
}

// 型エラー
try {
    int value = j.get<int>();  // jが文字列の場合
} catch (json::type_error& e) {
    std::cerr << "Type error: " << e.what() << std::endl;
}

// 範囲外エラー
try {
    json value = j.at(100);  // インデックス範囲外
} catch (json::out_of_range& e) {
    std::cerr << "Out of range: " << e.what() << std::endl;
}

// 無効なイテレータエラー
try {
    json::iterator it = j.begin();
    j.erase(it);
    *it;  // 無効なイテレータ
} catch (json::invalid_iterator& e) {
    std::cerr << "Invalid iterator: " << e.what() << std::endl;
}

// その他のエラー
try {
    // 何らかの操作
} catch (json::other_error& e) {
    std::cerr << "Other error: " << e.what() << std::endl;
}
```

---

## よく使うパターン

### 設定ファイルの読み込み

```cpp
std::ifstream config_file("config.json");
if (!config_file.is_open()) {
    throw std::runtime_error("Cannot open config.json");
}

json config;
try {
    config_file >> config;
} catch (json::parse_error& e) {
    std::cerr << "Config parse error: " << e.what() << std::endl;
    return;
}

// 設定値の取得（デフォルト値付き）
std::string server_host = config.value("server", json{{"host", "localhost"}})["host"];
int server_port = config.value("server", json{{"port", 8080}})["port"];
```

### データの検証

```cpp
bool ValidateUserData(const json& user) {
    if (!user.is_object()) {
        return false;
    }
    
    if (!user.contains("name") || !user["name"].is_string()) {
        return false;
    }
    
    if (!user.contains("age") || !user["age"].is_number_integer()) {
        return false;
    }
    
    int age = user["age"];
    if (age < 0 || age > 150) {
        return false;
    }
    
    return true;
}
```

### 配列の操作

```cpp
json users = json::array();

// ユーザーの追加
users.push_back({{"name", "Alice"}, {"age", 25}});
users.push_back({{"name", "Bob"}, {"age", 30}});

// フィルタリング
json young_users = json::array();
for (auto& user : users) {
    if (user["age"] < 30) {
        young_users.push_back(user);
    }
}

// マッピング
json names = json::array();
for (auto& user : users) {
    names.push_back(user["name"]);
}
```

### オブジェクトのマージ

```cpp
json base = {{"a", 1}, {"b", 2}};
json update = {{"b", 3}, {"c", 4}};

// update()でマージ
base.update(update);
// {"a": 1, "b": 3, "c": 4}

// merge_patch()でマージ
json base2 = {{"a", 1}, {"b", 2}};
json patch = {{"b", 3}, {"c", 4}};
base2.merge_patch(patch);
// {"a": 1, "b": 3, "c": 4}
```

### 深いコピー

```cpp
json original = {{"nested", {{"value", 42}}}};

// コピーコンストラクタ
json copy = original;

// 変更しても元に影響しない
copy["nested"]["value"] = 100;
// original["nested"]["value"] は 42 のまま
```

### 条件付きアクセス

```cpp
json data = {{"optional", "value"}};

// 安全なアクセス
if (data.contains("optional")) {
    std::string value = data["optional"];
}

// デフォルト値付きアクセス
std::string value = data.value("optional", "default");

// JSON Pointerを使用
std::string value = data.value("/nested/key"_json_pointer, "default");
```

---

## 検索インデックス

### カテゴリ別検索

- **作成**: `json::parse()`, `json::array()`, `json::object()`, 初期化リスト
- **アクセス**: `operator[]`, `at()`, `value()`, `get()`, `get_to()`
- **型チェック**: `is_null()`, `is_object()`, `is_array()`, `is_string()`, `is_number()`
- **型変換**: `get<T>()`, `get_to()`, `template get<T>()`
- **イテレータ**: `begin()`, `end()`, `items()`, `rbegin()`, `rend()`
- **コンテナ操作**: `push_back()`, `emplace_back()`, `erase()`, `insert()`, `clear()`
- **検索**: `find()`, `contains()`, `count()`
- **シリアライゼーション**: `dump()`, `operator<<`, `operator>>`
- **パッチ**: `patch()`, `merge_patch()`, `json::diff()`
- **JSON Pointer**: `json_pointer`, `operator[]` with `_json_pointer`
- **バイナリ**: `to_bson()`, `from_bson()`, `to_cbor()`, `from_cbor()`, `to_msgpack()`, `from_msgpack()`

### よく使う関数

- **パース**: `json::parse()`, `"_json"` リテラル
- **シリアライゼーション**: `dump()`, `operator<<`
- **アクセス**: `operator[]`, `at()`, `value()`
- **型変換**: `get<T>()`, `get_to()`
- **存在確認**: `contains()`, `find()`
- **配列操作**: `push_back()`, `emplace_back()`, `erase()`
- **イテレーション**: `begin()`, `end()`, `items()`

---

**注意**: このチートシートは nlohmann/json の主要な機能をまとめています。詳細なパラメータや使用例については、[nlohmann/json公式ドキュメント](https://json.nlohmann.me/)を参照してください。

