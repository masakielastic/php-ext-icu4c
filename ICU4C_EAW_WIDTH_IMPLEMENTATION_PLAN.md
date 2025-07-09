# icu4c_eaw_width 関数実装計画

## 概要

`icu4c_eaw_width` 関数を実装し、単一の文字を引数として受け取り、その文字の表示幅（display width）を計算して返すPHP拡張機能を作成する。Unicode Standard Annex #11の推奨事項に従って実装する。

## 技術仕様

### 関数シグネチャ
```php
function icu4c_eaw_width(string $char, ?string $locale = null): int
```

### 引数
- `$char` (string): 単一の文字（UTF-8エンコード）
- `$locale` (string|null): オプション。ロケール文字列（例: "ja", "zh", "ko"）。Ambiguous文字の幅判定に使用

### 戻り値
- `int`: 文字の表示幅
  - `1` - 半角幅（narrow/halfwidth）
  - `2` - 全角幅（wide/fullwidth）

## 実装計画

### 1. ICU4C East Asian Width プロパティの理解

#### 1.1 Unicode Technical Report #11 (UAX #11)
- East Asian Width プロパティは Unicode 標準の一部
- 6つの値を定義：N, A, H, F, Na, W
- 東アジア言語での文字レンダリングに重要

#### 1.2 ICU4C の実装
```c
#include <unicode/uchar.h>
#include <unicode/utypes.h>

UEastAsianWidth width = (UEastAsianWidth)u_getIntPropertyValue(codepoint, UCHAR_EAST_ASIAN_WIDTH);
```

### 2. 関数実装設計

#### 2.1 基本構造（sample.phpのアルゴリズムをベースに）
```c
PHP_FUNCTION(icu4c_eaw_width)
{
    zend_string *input;
    zend_string *locale = NULL;
    
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(input)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(locale)
    ZEND_PARSE_PARAMETERS_END();
    
    // 文字列から最初のUnicode文字を取得
    UChar32 codepoint = icu4c_get_first_codepoint(ZSTR_VAL(input), ZSTR_LEN(input));
    
    if (codepoint < 0) {
        RETURN_FALSE;
    }
    
    // East Asian Width プロパティを取得
    UEastAsianWidth eaw = (UEastAsianWidth)u_getIntPropertyValue(codepoint, UCHAR_EAST_ASIAN_WIDTH);
    
    // 表示幅を計算
    int width = icu4c_calculate_display_width(eaw, locale);
    
    RETURN_LONG(width);
}
```

#### 2.2 ヘルパー関数
```c
// UTF-8文字列から最初のUnicode文字を取得
UChar32 icu4c_get_first_codepoint(const char *str, size_t len);

// East Asian Width プロパティから表示幅を計算
int icu4c_calculate_display_width(UEastAsianWidth eaw, zend_string *locale);

// ロケールが東アジア系かどうかを判定
bool icu4c_is_east_asian_locale(const char *locale);
```

### 3. East Asian Width プロパティマッピング

#### 3.1 ICU4C 列挙値
```c
typedef enum UEastAsianWidth {
    U_EA_NEUTRAL = 0,     // [N]  - 中立
    U_EA_AMBIGUOUS = 1,   // [A]  - 曖昧
    U_EA_HALFWIDTH = 2,   // [H]  - 半角
    U_EA_FULLWIDTH = 3,   // [F]  - 全角
    U_EA_NARROW = 4,      // [Na] - 狭い
    U_EA_WIDE = 5         // [W]  - 広い
} UEastAsianWidth;
```

#### 3.2 Unicode Standard Annex #11 準拠の表示幅計算
```c
int icu4c_calculate_display_width(UEastAsianWidth eaw, zend_string *locale)
{
    switch (eaw) {
        case U_EA_FULLWIDTH:  // [F] 全角
        case U_EA_WIDE:       // [W] 広い
            return 2;
        
        case U_EA_AMBIGUOUS:  // [A] 曖昧（コンテキスト依存）
            if (locale && icu4c_is_east_asian_locale(ZSTR_VAL(locale))) {
                return 2;  // 東アジア系ロケールでは全角扱い
            }
            return 1;  // その他では半角扱い
        
        case U_EA_HALFWIDTH:  // [H] 半角
        case U_EA_NARROW:     // [Na] 狭い
        case U_EA_NEUTRAL:    // [N] 中立
        default:
            return 1;
    }
}

bool icu4c_is_east_asian_locale(const char *locale)
{
    if (!locale) return false;
    
    // 東アジア系ロケールの判定（sample.phpに準拠）
    return (strncmp(locale, "ja", 2) == 0 ||  // 日本語
            strncmp(locale, "zh", 2) == 0 ||  // 中国語
            strncmp(locale, "ko", 2) == 0);   // 韓国語
}
```

### 4. 実装手順

#### 4.1 フェーズ1: 基本関数実装
1. `php_icu4c.h` に関数宣言を追加
2. `icu4c.c` に `icu4c_eaw_width` 関数実装
3. ArgInfo 定義の追加
4. 関数エントリーの登録

#### 4.2 フェーズ2: ヘルパー関数実装
1. UTF-8 から UChar32 への変換関数
2. East Asian Width 列挙値から文字列への変換関数
3. エラーハンドリングの実装

#### 4.3 フェーズ3: テストと検証
1. 各East Asian Width値のテストケース作成
2. 複合文字の処理テスト
3. エラーケースの検証

### 5. 文字幅の分類と用途

#### 5.1 文字幅分類
- **半角相当**: `Narrow`, `Halfwidth`
- **全角相当**: `Wide`, `Fullwidth`
- **コンテキスト依存**: `Ambiguous`
- **中立**: `Neutral`

#### 5.2 実用的な用途
- ターミナル表示でのカラム幅計算
- テキストエディタでの文字配置
- 東アジア言語での行折り返し処理

### 6. テストケース設計

#### 6.1 基本文字テスト
```php
// ASCII文字（Neutral）
echo icu4c_eaw_width('A');     // 1

// 全角文字（Wide）
echo icu4c_eaw_width('あ');    // 2
echo icu4c_eaw_width('漢');    // 2

// 半角カナ（Halfwidth）
echo icu4c_eaw_width('ｱ');     // 1

// 全角英数（Fullwidth）
echo icu4c_eaw_width('Ａ');    // 2
```

#### 6.2 Ambiguous文字のロケール依存テスト
```php
// 記号（Ambiguous）
echo icu4c_eaw_width('±');        // 1（デフォルト）
echo icu4c_eaw_width('±', 'ja');  // 2（日本語ロケール）
echo icu4c_eaw_width('±', 'en');  // 1（英語ロケール）

// ギリシャ文字（Ambiguous）
echo icu4c_eaw_width('α');        // 1（デフォルト）
echo icu4c_eaw_width('α', 'zh');  // 2（中国語ロケール）
```

#### 6.3 特殊ケーステスト
```php
// 絵文字（Wide）
echo icu4c_eaw_width('😀');    // 2

// 制御文字（Neutral）
echo icu4c_eaw_width("\t");    // 1
```

### 7. エラーハンドリング

#### 7.1 入力検証
- 空文字列の場合は `FALSE` を返す
- 不正なUTF-8シーケンスの場合は `FALSE` を返す
- 複数文字の場合は最初の文字のみを処理

#### 7.2 ICU4C エラー
- 無効なコードポイントの場合は `"Unknown"` を返す
- ICU4Cが利用できない場合のフォールバック処理

### 8. 使用例

#### 8.1 基本的な使用
```php
$char = 'あ';
$width = icu4c_eaw_width($char);
echo "文字 '$char' の表示幅: {$width}\n";
// 出力: 文字 'あ' の表示幅: 2

// ロケール指定
$char = '±';
$width_default = icu4c_eaw_width($char);
$width_ja = icu4c_eaw_width($char, 'ja');
echo "文字 '$char' の表示幅（デフォルト）: {$width_default}\n";  // 1
echo "文字 '$char' の表示幅（日本語）: {$width_ja}\n";          // 2
```

#### 8.2 実用的な応用（sample.phpと互換）
```php
function icu4c_strwidth(string $str, ?string $locale = null): int
{
    $width = 0;
    $iterator = icu4c_iter($str);
    
    foreach ($iterator as $char) {
        $width += icu4c_eaw_width($char, $locale);
    }
    
    return $width;
}

// 使用例
$text = "Hello世界";
echo icu4c_strwidth($text);        // 9 (5 + 4)
echo icu4c_strwidth($text, 'ja');  // 9 (5 + 4)

$text = "±±±";
echo icu4c_strwidth($text);        // 3 (1 + 1 + 1)
echo icu4c_strwidth($text, 'ja');  // 6 (2 + 2 + 2)
```

### 9. 既存実装との統合

#### 9.1 既存の拡張への追加
- 既存の `config.m4` を拡張（ICU4C依存関係は既に設定済み）
- `php_icu4c.h` に新しい関数宣言を追加
- `icu4c.c` に実装を追加

#### 9.2 `icu4c_iter` との組み合わせ
```php
$text = "Hello世界±";
$iterator = icu4c_iter($text);

foreach ($iterator as $char) {
    $width = icu4c_eaw_width($char, 'ja');
    echo "文字: '$char', 幅: $width\n";
}
// 出力:
// 文字: 'H', 幅: 1
// 文字: 'e', 幅: 1
// 文字: 'l', 幅: 1
// 文字: 'l', 幅: 1
// 文字: 'o', 幅: 1
// 文字: '世', 幅: 2
// 文字: '界', 幅: 2
// 文字: '±', 幅: 2
```

### 10. 参考資料

- [Unicode Technical Report #11 (UAX #11)](https://www.unicode.org/reports/tr11/)
- [Unicode Standard Annex #11 - Section 5 Recommendations](https://www.unicode.org/reports/tr11/#Recommendations)
- [ICU4C Unicode Property API](https://unicode-org.github.io/icu/userguide/strings/properties.html)
- [East Asian Width Property Values](https://www.unicode.org/reports/tr11/#ED1)
- `sample.php` - 既存のIntlChar実装との互換性確保

## 成果物

1. `icu4c_eaw_width` 関数の実装（数値戻り値、ロケール対応）
2. Unicode Standard Annex #11準拠の文字幅計算
3. `sample.php`と互換性のある文字幅計算関数
4. 東アジア系ロケールでのAmbiguous文字処理
5. 包括的なテストケースとドキュメント