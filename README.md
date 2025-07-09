# PHP ICU4C Extension

A PHP extension that provides accurate Unicode grapheme cluster iteration using ICU4C (International Components for Unicode for C/C++).

## Features

- **Accurate Grapheme Cluster Detection**: Uses ICU4C's BreakIterator for precise Unicode text segmentation
- **Iterator Pattern**: Implements `IteratorAggregate` and `Countable` interfaces
- **Complex Unicode Support**: Handles emoji sequences, combining characters, and variation selectors
- **East Asian Width Calculation**: Calculates display width based on Unicode EAW properties for proper text alignment
- **Fallback Support**: Graceful degradation to UTF-8 character processing when ICU4C is unavailable

## Requirements

- PHP 8.0 or higher
- ICU4C library (libicu-dev on Ubuntu/Debian)
- PHP development headers (php-dev)

## Installation

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt-get install libicu-dev php-dev

# Build the extension
phpize
./configure --with-icu4c
make
```

### Usage

```php
<?php
// Load the extension
extension_loaded('icu4c') or dl('icu4c.so');

// Create iterator for complex Unicode text
$text = "葛\u{E0101}飾区";  // Text with variation selector
$iterator = icu4c_iter($text);

// Count grapheme clusters
echo "Total clusters: " . count($iterator) . "\n";  // Output: 3

// Iterate through grapheme clusters
foreach ($iterator as $index => $cluster) {
    echo "[$index]: $cluster\n";
}
// Output:
// [0]: 葛󠄁
// [1]: 飾
// [2]: 区

// Use Iterator interface directly
$iterator->rewind();
while ($iterator->valid()) {
    echo $iterator->current() . " ";
    $iterator->next();
}
```

## API Reference

### Functions

#### `icu4c_iter(string $text): ICU4CIterator`

Creates an iterator for the given text that segments it into grapheme clusters.

**Parameters:**
- `$text` (string): The input text to iterate over

**Returns:**
- `ICU4CIterator`: An iterator object implementing `IteratorAggregate` and `Countable`

#### `icu4c_eaw_width(string $text): int`

Calculates the display width of text based on East Asian Width (EAW) properties according to Unicode Standard Annex #11.

**Parameters:**
- `$text` (string): The input text to calculate width for

**Returns:**
- `int`: The total display width of the text

**Width Calculation:**
- **Wide (W)** and **Fullwidth (F)** characters: 2 units
- **Narrow (Na)**, **Halfwidth (H)**, and **Ambiguous (A)** characters: 1 unit
- **Neutral (N)** characters: 1 unit

This function is particularly useful for:
- Terminal/console applications requiring proper text alignment
- Monospace font environments
- CJK (Chinese, Japanese, Korean) text processing
- Fixed-width layout calculations

### ICU4CIterator Class

Implements the following interfaces:
- `IteratorAggregate`
- `Countable`
- `Iterator`

#### Methods

- `count(): int` - Returns the number of grapheme clusters
- `getIterator(): Iterator` - Returns the iterator (self)
- `current(): string` - Returns the current grapheme cluster
- `key(): int` - Returns the current position
- `next(): void` - Advances to the next grapheme cluster
- `rewind(): void` - Resets the iterator to the beginning
- `valid(): bool` - Checks if the current position is valid

## Examples

### Basic Usage

```php
$iterator = icu4c_iter("Hello");
echo count($iterator); // 5

foreach ($iterator as $char) {
    echo $char . " ";
}
// Output: H e l l o
```

### Complex Unicode Text

```php
// Text with combining characters
$text = "café";
$iterator = icu4c_iter($text);
echo count($iterator); // 4 (not 5, because é is one grapheme cluster)

// Emoji family sequence
$emoji = "👨‍👩‍👧‍👦";
$iterator = icu4c_iter($emoji);
echo count($iterator); // 1 (entire family is one grapheme cluster)
```

### East Asian Width Calculation

```php
// ASCII text
$text = "Hello";
echo icu4c_eaw_width($text); // 5

// Japanese text with wide characters
$text = "こんにちは";
echo icu4c_eaw_width($text); // 10 (5 characters × 2 units each)

// Mixed text
$text = "Hello世界";
echo icu4c_eaw_width($text); // 9 (5 + 4)

// Terminal alignment example
$name = "田中太郎";
$padding = 20 - icu4c_eaw_width($name);
echo $name . str_repeat(' ', $padding) . "| End\n";
```

### Iterator Interface

```php
$iterator = icu4c_iter("ABC");
$iterator->rewind();

while ($iterator->valid()) {
    echo "Position: " . $iterator->key() . ", Value: " . $iterator->current() . "\n";
    $iterator->next();
}
```

## Technical Details

### ICU4C Integration

The extension uses ICU4C's `UBreakIterator` with `UBRK_CHARACTER` mode to identify grapheme cluster boundaries. This ensures accurate processing of:

- Combining character sequences
- Emoji modifier sequences
- Variation selectors
- Complex script characters

### Fallback Behavior

When ICU4C is not available, the extension falls back to UTF-8 character processing, which handles basic multibyte characters but may not correctly process complex grapheme clusters.

### Memory Management

The extension properly manages ICU4C resources:
- `UBreakIterator` objects are properly closed
- `UText` objects are released
- Boundary arrays are allocated and freed appropriately

## Testing

Run the test suite:

```bash
php -dextension=./modules/icu4c.so test.php
```

## Contributing

This project follows [Conventional Commits](https://conventionalcommits.org/) for commit messages.

## License

This extension is provided as-is for educational and development purposes.

## See Also

- [ICU4C Documentation](https://unicode-org.github.io/icu/)
- [Unicode Text Segmentation](https://unicode.org/reports/tr29/)
- [PHP Extension Development](https://www.php.net/manual/en/internals2.php)