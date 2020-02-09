module Main where
import Diagrams.Backend.Rasterific.CmdLine
import Diagrams.Prelude
import Hilbert

fractal :: Diagram B
fractal = strokeFractal (peano 4 `at` origin) # frame 1 # lwL 1

main = mainWith fractal
