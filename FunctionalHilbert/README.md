# Functional Hilbert
This is basic haskell realization of hilbert curve, using Diagrams package.
Has coloured and non coloured drawing function.

--to add animation - [just open pdf :)](https://github.com/kanashimia/Throw-Away-Projects/blob/master/FunctionalHilbert/h-animated_compressed.pdf)

### This is basically whole code for calculating hilbert curve:

```
hilbert :: Int -> Trail V2 Double
hilbert 0 = mempty
hilbert n = hilbert (n-1)  # rotateBy (1/4) # reflectY <> vrule 1
         <> hilbert (n-1) <> hrule 1
         <> hilbert (n-1) <> vrule (-1)
         <> hilbert (n-1)  # rotateBy (1/4) # reflectX
```
### This is whole coloured drawing:
```
drawHilbertColor n = (hilbert n `at` origin)
    # explodeTrail # map (lineCap LineCapRound . strokeLocT)
    # (zipWith lc =<< colors . length) # mconcat
  where
    colors n = cycle $ map makeColor [0,360 / realToFrac n..360]
    makeColor a = uncurryRGB sRGB $ hsv a 1 1
© 2020 GitHub, Inc.
```

## Output

<table class="tg">
  <tr>
    <th class="tg-yw4l"><img src="h-small.png"></th>
    <th class="tg-yw4l"><img src="h-animated.png"></th>
    <th class="tg-yw4l"><img src="h.png"></th>
  </tr>

</table>

For other file formats check files named h.* in main repo folder


