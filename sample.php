<?php

function intl_strwidth(string $str, ?string $locale = null): int
{
    $width = 0;
    $len = grapheme_strlen($str);
    for ($i = 0; $i < $len; $i++) {
        $grapheme = grapheme_substr($str, $i, 1);
        $codepoint = IntlChar::ord($grapheme);
        $eaw = IntlChar::getIntPropertyValue($codepoint, IntlChar::PROPERTY_EAST_ASIAN_WIDTH);
        switch ($eaw) {
            case IntlChar::EAST_ASIAN_WIDTH_FULLWIDTH:
            case IntlChar::EAST_ASIAN_WIDTH_WIDE:
                $w = 2;
                break;
            case IntlChar::EAST_ASIAN_WIDTH_AMBIGUOUS:
                $w = (preg_match('/^(ja|zh|ko)/', $locale ?? getenv('LANG'))) ? 2 : 1;
                break;
            default:
                $w = 1;
        }
        $width += $w;
    }
    return $width;
}

