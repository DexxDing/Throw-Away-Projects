{-# LANGUAGE FlexibleContexts, Rank2Types #-}
module Peano where
import Diagrams.Prelude
import Data.Colour.RGBSpace
import Data.Colour.RGBSpace.HSV


type Fractal = Trail V2 Double
type DiagramFractal b = Renderable (Path V2 Double) b
                     => QDiagram b V2 Double Any


hilbert :: Int -> Fractal
hilbert 0 = mempty
hilbert n = hilbert (n-1)  # rotateBy (1/4) # reflectY <> vrule 1
         <> hilbert (n-1) <> hrule 1
         <> hilbert (n-1) <> vrule (-1)
         <> hilbert (n-1)  # rotateBy (1/4) # reflectX


peano :: Int -> Fractal
peano 0 = mempty
peano n = strip <> hrule 1
       <> strip # reflectY <> hrule 1
       <> strip
  where
    strip = peano (n-1) <> vrule 1
         <> peano (n-1)  # reflectX <> vrule 1
         <> peano (n-1)


strokeFractal :: Located Fractal -> DiagramFractal b
strokeFractal = mconcat . colourize . explodeTrail
  where
    colourize   = (zipWith lc =<< colors . length)
                . map (lineCap LineCapSquare . strokeLocT)
    colors len  = cycle $ map makeColor [0, 360/realToFrac len..360]
    makeColor a = uncurryRGB sRGB $ hsv a 1 1
