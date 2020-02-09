{-# LANGUAGE FlexibleContexts #-}
module Hilbert where
import Diagrams.Prelude
import Data.Colour.RGBSpace
import Data.Colour.RGBSpace.HSV

hilbert :: Int -> Trail V2 Double
hilbert 0 = mempty
hilbert n = hilbert (n-1)  # rotateBy (1/4) # reflectY <> vrule 1
         <> hilbert (n-1) <> hrule 1
         <> hilbert (n-1) <> vrule (-1)
         <> hilbert (n-1)  # rotateBy (1/4) # reflectX


drawHilbert n = hilbert n # strokeT # lc white

drawHilbertColor n = (hilbert n `at` origin)
    # explodeTrail # map (lineCap LineCapRound . strokeLocT)
    # (zipWith lc =<< colors . length) # mconcat
  where
    colors n = cycle $ map makeColor [0,360 / realToFrac n..360]
    makeColor a = uncurryRGB sRGB $ hsv a 1 1
