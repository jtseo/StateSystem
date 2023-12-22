del /Q ..\PrototypeSii\Assets\StreamingAssets\State\*.*
del /Q ..\PrototypeSii\Assets\VLogger\Resources\*.*
copy *.data ..\PrototypeSii\Assets\StreamingAssets\State
del ..\PrototypeSii\Assets\StreamingAssets\State\*Pos.data
copy *.define ..\PrototypeSii\Assets\StreamingAssets\State
copy ..\PrototypeSii\Assets\StreamingAssets\State\*.* ..\PrototypeSii\Assets\VLogger\Resources
ren ..\PrototypeSii\Assets\VLogger\Resources\*.data *.data.bytes
ren ..\PrototypeSii\Assets\VLogger\Resources\*.define *.define.bytes