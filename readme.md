# build steps
install QT.  
This project was built using Qt 6.11.1, other versions may not work.  
Using QT Online Installer, once installed:  

MacOs  
In the same directory as this readme, run:  
```cmake -B build -DCMAKE_PREFIX_PATH="$HOME/Qt/{YOUR QT VERSION}/macos"```  


Windows  
In the same directory as this readme, run:  
```cmake -B build -DCMAKE_PREFIX_PATH="PATH/TO/QT/Qt/{YOUR QT VERSION}/mingw_64"```

Then you should be able to build.  