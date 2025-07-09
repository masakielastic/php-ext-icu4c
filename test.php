<?php

// Test script for icu4c_iter function

echo "Testing icu4c_iter function\n";
echo "==========================\n\n";

// Test 1: Basic ASCII text
echo "Test 1: Basic ASCII text\n";
$text1 = "Hello";
$iter1 = icu4c_iter($text1);
echo "Text: '$text1'\n";
echo "Count: " . count($iter1) . "\n";
echo "Characters: ";
foreach ($iter1 as $i => $char) {
    echo "[$i]='$char' ";
}
echo "\n\n";

// Test 2: UTF-8 text with combining characters
echo "Test 2: UTF-8 text with combining characters\n";
$text2 = "café";
$iter2 = icu4c_iter($text2);
echo "Text: '$text2'\n";
echo "Count: " . count($iter2) . "\n";
echo "Characters: ";
foreach ($iter2 as $i => $char) {
    echo "[$i]='$char' ";
}
echo "\n\n";

// Test 3: Complex grapheme clusters (from sample.c)
echo "Test 3: Complex grapheme clusters\n";
$text3 = "葛\u{E0101}飾区";
$iter3 = icu4c_iter($text3);
echo "Text: '$text3'\n";
echo "Count: " . count($iter3) . "\n";
echo "Characters: ";
foreach ($iter3 as $i => $char) {
    echo "[$i]='$char' ";
}
echo "\n\n";

// Test 4: Empty string
echo "Test 4: Empty string\n";
$text4 = "";
$iter4 = icu4c_iter($text4);
echo "Text: '$text4'\n";
echo "Count: " . count($iter4) . "\n";
echo "Characters: ";
foreach ($iter4 as $i => $char) {
    echo "[$i]='$char' ";
}
echo "\n\n";

// Test 5: Emoji with modifiers
echo "Test 5: Emoji with modifiers\n";
$text5 = "👨‍👩‍👧‍👦";
$iter5 = icu4c_iter($text5);
echo "Text: '$text5'\n";
echo "Count: " . count($iter5) . "\n";
echo "Characters: ";
foreach ($iter5 as $i => $char) {
    echo "[$i]='$char' ";
}
echo "\n\n";

// Test 6: Iterator interface methods
echo "Test 6: Iterator interface methods\n";
$text6 = "ABC";
$iter6 = icu4c_iter($text6);
echo "Text: '$text6'\n";
echo "Using Iterator methods:\n";
$iter6->rewind();
while ($iter6->valid()) {
    echo "Key: " . $iter6->key() . ", Value: '" . $iter6->current() . "'\n";
    $iter6->next();
}
echo "\n";

// Test 7: IteratorAggregate interface
echo "Test 7: IteratorAggregate interface\n";
$text7 = "XYZ";
$iter7 = icu4c_iter($text7);
echo "Text: '$text7'\n";
$internal_iter = $iter7->getIterator();
echo "getIterator() returns: " . get_class($internal_iter) . "\n";
echo "Is same object: " . ($internal_iter === $iter7 ? "yes" : "no") . "\n";
echo "\n";

echo "All tests completed.\n";
?>