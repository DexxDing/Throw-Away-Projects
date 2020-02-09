module Main where
import Diagrams.Backend.Rasterific.CmdLine
import Diagrams.Prelude
import Hilbert

main = mainWith (drawHilbertColor 7 # frame 1 :: Diagram B)
