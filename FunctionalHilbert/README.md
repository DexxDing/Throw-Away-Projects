# Functional Hilbert (and Peano) Curve
This is the basic implementation of the Hilbert curve in haskell using the Diagrams package.

--to add animation - [just open pdf :)](https://github.com/kanashimia/Throw-Away-Projects/blob/master/FunctionalHilbert/h-animated_compressed.pdf) or [another pdf](https://github.com/kanashimia/Throw-Away-Projects/blob/master/FunctionalHilbert/peano-animated_compressed.pdf)

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
strokeFractal :: Located Fractal -> DiagramFractal b
strokeFractal = mconcat . colourize . explodeTrail
  where
    colourize   = (zipWith lc =<< colors . length)
                . map (lineCap LineCapSquare . strokeLocT)
    colors len  = cycle $ map makeColor [0, 360/realToFrac len..360]
    makeColor a = uncurryRGB sRGB $ hsv a 1 1
```

## Output

<table class="tg">
  <tr>
    <th class="tg-yw4l"><img src="h-small.png"></th>
    <th class="tg-yw4l"><img src="h-animated.png"></th>
    <th class="tg-yw4l"><img src="peano-small.png"></th>
  </tr>

</table>

For other file formats check files named **[REDACTED]** in the main folder

## Author's small talk (i dont know smalltalk tho):

Before code was smaller, but i decided to add some redibility.
Alignment isn't necessary (it's actually an extremely bad habit) but i think it looks better that way.

I've been using this package for literally 1 day, so i am sure that code can be smaller.

<br/>
PDF animation is 200iq; big brain time.
