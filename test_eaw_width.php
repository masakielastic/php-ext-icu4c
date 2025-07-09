<?php

// Test script for icu4c_eaw_width function

echo "Testing icu4c_eaw_width function\n";
echo "================================\n\n";

// Test 1: Basic ASCII characters (Neutral)
echo "Test 1: ASCII characters (Neutral)\n";
$chars = ['A', 'B', '1', '2', ' ', '!'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 2: Wide characters (Japanese/Chinese)
echo "Test 2: Wide characters (Japanese/Chinese)\n";
$chars = ['あ', 'い', 'う', '漢', '字', '中', '国'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 3: Halfwidth characters
echo "Test 3: Halfwidth characters\n";
$chars = ['ｱ', 'ｲ', 'ｳ', 'ｴ', 'ｵ'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 4: Fullwidth characters
echo "Test 4: Fullwidth characters\n";
$chars = ['Ａ', 'Ｂ', 'Ｃ', '１', '２', '３'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 5: Ambiguous characters without locale
echo "Test 5: Ambiguous characters (no locale)\n";
$chars = ['±', '×', '÷', '°', '§', 'α', 'β', 'γ'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 6: Ambiguous characters with East Asian locale
echo "Test 6: Ambiguous characters (Japanese locale)\n";
$chars = ['±', '×', '÷', '°', '§', 'α', 'β', 'γ'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char, 'ja');
    echo "'{$char}' (ja) -> {$width}\n";
}
echo "\n";

// Test 7: Ambiguous characters with Chinese locale
echo "Test 7: Ambiguous characters (Chinese locale)\n";
$chars = ['±', '×', '÷', '°', '§'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char, 'zh');
    echo "'{$char}' (zh) -> {$width}\n";
}
echo "\n";

// Test 8: Ambiguous characters with Korean locale
echo "Test 8: Ambiguous characters (Korean locale)\n";
$chars = ['±', '×', '÷', '°', '§'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char, 'ko');
    echo "'{$char}' (ko) -> {$width}\n";
}
echo "\n";

// Test 9: Ambiguous characters with English locale
echo "Test 9: Ambiguous characters (English locale)\n";
$chars = ['±', '×', '÷', '°', '§'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char, 'en');
    echo "'{$char}' (en) -> {$width}\n";
}
echo "\n";

// Test 10: Emoji characters (should be Wide)
echo "Test 10: Emoji characters\n";
$chars = ['😀', '😁', '🎉', '🌟', '❤️'];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    echo "'{$char}' -> {$width}\n";
}
echo "\n";

// Test 11: Control characters
echo "Test 11: Control characters\n";
$chars = ["\t", "\n", "\r"];
foreach ($chars as $char) {
    $width = icu4c_eaw_width($char);
    $display = addcslashes($char, "\t\n\r");
    echo "'{$display}' -> {$width}\n";
}
echo "\n";

// Test 12: Error cases
echo "Test 12: Error cases\n";
$result = icu4c_eaw_width('');
echo "Empty string -> " . ($result === false ? "false" : $result) . "\n";
echo "\n";

// Test 13: Integration with icu4c_iter
echo "Test 13: Integration with icu4c_iter\n";
$text = "Hello世界±";
$iterator = icu4c_iter($text);
$total_width = 0;

echo "Text: '{$text}'\n";
echo "Character analysis:\n";
foreach ($iterator as $index => $char) {
    $width_default = icu4c_eaw_width($char);
    $width_ja = icu4c_eaw_width($char, 'ja');
    $total_width += $width_default;
    echo "  [{$index}] '{$char}' -> width: {$width_default}, width(ja): {$width_ja}\n";
}
echo "Total width (default): {$total_width}\n";

// Calculate total width with Japanese locale
$total_width_ja = 0;
foreach ($iterator as $char) {
    $total_width_ja += icu4c_eaw_width($char, 'ja');
}
echo "Total width (ja): {$total_width_ja}\n";
echo "\n";

// Test 14: Sample compatibility test
echo "Test 14: Compatibility with sample.php logic\n";
function icu4c_strwidth(string $str, ?string $locale = null): int
{
    $width = 0;
    $iterator = icu4c_iter($str);
    
    foreach ($iterator as $char) {
        $width += icu4c_eaw_width($char, $locale);
    }
    
    return $width;
}

$test_strings = [
    "Hello",
    "世界",
    "Hello世界",
    "±±±",
    "αβγ",
    "Ａ Ｂ Ｃ",
    "ｱｲｳｴｵ"
];

foreach ($test_strings as $str) {
    $width_default = icu4c_strwidth($str);
    $width_ja = icu4c_strwidth($str, 'ja');
    echo "'{$str}' -> width: {$width_default}, width(ja): {$width_ja}\n";
}

echo "\nAll tests completed.\n";
?>