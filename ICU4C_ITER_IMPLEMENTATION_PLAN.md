# icu4c_iter 関数実装計画

## 概要

`icu4c_iter` 関数を実装し、文字列引数を受け取って書記素クラスター（grapheme cluster）のイテレーターを返すPHP拡張機能を作成する。

## 技術仕様

### 関数シグネチャ
```php
function icu4c_iter(string $text): ICU4CIterator
```

### 戻り値
- `ICU4CIterator` クラスのインスタンス
- `IteratorAggregate` インターフェースを実装
- `Countable` インターフェースを実装

## 実装計画

### 1. 基本構造設計

#### 1.1 PHP拡張の基本ファイル
- `config.m4` - ビルド設定
- `php_icu4c.h` - ヘッダファイル
- `icu4c.c` - メイン実装
- `icu4c_iterator.c` - イテレータークラス実装

#### 1.2 依存関係
- ICU4C ライブラリ（libicu-dev）
- PHP 開発ヘッダー（php-dev）

### 2. ICU4CIterator クラス設計

#### 2.1 オブジェクト構造
```c
typedef struct _icu4c_iterator_obj {
    zend_string *text;           // 元の文字列
    UBreakIterator *break_iter;  // ICU4C BreakIterator
    UText *utext;               // ICU4C UText
    size_t current_pos;         // 現在位置
    size_t total_clusters;      // 総クラスター数
    zend_object std;            // 標準オブジェクト
} icu4c_iterator_obj;
```

#### 2.2 実装インターフェース
- `IteratorAggregate::getIterator()` - 自身を返す
- `Countable::count()` - 書記素クラスター数を返す
- `Iterator::current()` - 現在の書記素クラスターを返す
- `Iterator::key()` - 現在のインデックスを返す
- `Iterator::next()` - 次の書記素クラスターに移動
- `Iterator::rewind()` - 最初に戻る
- `Iterator::valid()` - 有効な位置にいるかチェック

### 3. ICU4C BreakIterator 統合

#### 3.1 初期化処理
```c
// sample.c を参考にした実装
UErrorCode status = U_ZERO_ERROR;
UText *ut = utext_openUTF8(NULL, text, -1, &status);
UBreakIterator *bi = ubrk_open(UBRK_CHARACTER, uloc_getDefault(), NULL, 0, &status);
ubrk_setUText(bi, ut, &status);
```

#### 3.2 書記素クラスター取得
```c
// 現在位置から次の境界までの文字列を取得
int32_t start = ubrk_current(bi);
int32_t end = ubrk_next(bi);
if (end != UBRK_DONE) {
    size_t cluster_size = end - start;
    return zend_string_init(text + start, cluster_size, 0);
}
```

### 4. php-ext-striter の参考実装

#### 4.1 採用パターン
- オブジェクト作成とメモリ管理
- Iterator/IteratorAggregate の実装パターン
- Countable インターフェースの実装
- 文字列処理のユーティリティ関数

#### 4.2 改良点
- PCRE2 の代わりに ICU4C BreakIterator を使用
- より正確な書記素クラスター処理
- Unicode 正規化の考慮

### 5. 実装手順

#### 5.1 フェーズ1: 基本構造
1. `config.m4` でICU4C依存関係を設定
2. `php_icu4c.h` でクラス定義と関数宣言
3. `icu4c.c` で `icu4c_iter` 関数実装
4. 基本的なクラス登録

#### 5.2 フェーズ2: ICU4CIterator クラス
1. オブジェクト作成/破棄ハンドラー
2. `__construct` メソッド実装
3. ICU4C BreakIterator 初期化
4. 書記素クラスター数計算

#### 5.3 フェーズ3: イテレーター機能
1. `Iterator` インターフェース実装
2. `current()`, `key()`, `next()`, `rewind()`, `valid()` メソッド
3. 書記素クラスター取得ロジック

#### 5.4 フェーズ4: 追加インターフェース
1. `IteratorAggregate::getIterator()` 実装
2. `Countable::count()` 実装
3. エラーハンドリング強化

#### 5.5 フェーズ5: テストと最適化
1. 単体テスト作成
2. Unicode テストケース
3. メモリリーク検査
4. パフォーマンス最適化

### 6. エラーハンドリング

#### 6.1 ICU4C エラー
- `U_ZERO_ERROR` 状態チェック
- `U_FAILURE()` マクロでエラー検出
- 適切なPHP例外スロー

#### 6.2 メモリ管理
- `ubrk_close()` でBreakIterator解放
- `utext_close()` でUText解放
- `zend_string_release()` で文字列解放

### 7. 使用例

```php
$text = "葛\u{E0101}飾区";
$iterator = icu4c_iter($text);

// Countable として使用
echo count($iterator); // 書記素クラスター数

// foreach で反復
foreach ($iterator as $index => $cluster) {
    echo "[$index]: $cluster\n";
}

// Iterator として直接使用
$iterator->rewind();
while ($iterator->valid()) {
    echo $iterator->current() . "\n";
    $iterator->next();
}
```

### 8. 参考資料

- `sample.c` - ICU4C BreakIterator の使用方法
- `php-ext-striter` - Iterator パターンの実装
- ICU4C ドキュメント - BreakIterator API
- PHP拡張開発ドキュメント - インターフェース実装

## 成果物

1. コンパイル可能なPHP拡張
2. 完全なテストスイート  
3. 使用例とドキュメント
4. ICU4C を使用した正確な書記素クラスター処理