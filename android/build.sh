set -e
# echo an error message before exiting
trap 'echo "\"${BASH_COMMAND}\" command filed with exit code $?."' EXIT

PROJECT=${PROJECT:-"project"}
ARCH=${ARCH:-"ARM"}

mkdir -p src/com/seth/$PROJECT
mkdir -p obj
mkdir -p dex
mkdir -p res/values
mkdir -p assets
mkdir -p lib/armeabi-v7a
RAYLIB="/opt/raylib"

if [ $ARCH == "ARM" ]; then
    ARCH_NAME="armeabi-v7a"
fi

cp ${RAYLIB}/release/libs/android/${ARCH_NAME}/libraylib.a lib/${ARCH_NAME}



# Note: Install android-16 through Android Studio (Android SDK Platform 16)
PLATFORM=${PLATFORM:-"android-16"}
ANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN:-"/opt/android_toolchain_arm_api21"}
CC=${ANDROID_TOOLCHAIN}/bin/arm-linux-androideabi-gcc
INCLUDE_PATHS="-I. -I$RAYLIB/release/include -I$RAYLIB/src/external/android/native_app_glue"


LDFLAGS="-Wl,-soname,lib$PROJECT.so -Wl,--exclude-libs,libatomic.a -Wl,--build-id -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings -u ANativeActivity_onCreate -L. -Lobj"
if [ "$ARCH" == "ARM64" ]; then
    LDFLAGS="$LDFLAGS -Llib/arm64-v8a"
    CFLAGS="-std=c99 -march=arm64-v8a -ffunction-sections -funwind-tables -fstack-protector-strong -fPIC -Wall -Wa,--noexecstack -Wformat -Werror=format-security -no-canonical-prefixes -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=16"
fi
if [ "$ARCH" == "ARM" ]; then
    LDFLAGS="$LDFLAGS -Llib/armeabi-v7a"
    CFLAGS="-std=c99 -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -ffunction-sections -funwind-tables -fstack-protector-strong -fPIC -Wall -Wa,--noexecstack -Wformat -Werror=format-security -no-canonical-prefixes -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=16"
fi
LDLIBS="-lraylib -lnative_app_glue -llog -landroid -lEGL -lGLESv2 -lOpenSLES -latomic -lc -lm -ldl"

if [ ! -f project.keystore ]; then
    keytool -genkeypair -validity 1000 -dname "CN=seth,O=Android,C=ES" -keystore project.keystore -storepass 'whatever' -keypass 'mypass' -alias projectKey -keyalg RSA
fi

if [ ! -f obj/native_app_glue.o ]; then
    echo "native_app_glue.o"
    $CC -c $RAYLIB/src/external/android/native_app_glue/android_native_app_glue.c -o obj/native_app_glue.o $CFLAGS
    $ANDROID_TOOLCHAIN/bin/arm-linux-androideabi-ar rcs obj/libnative_app_glue.a obj/native_app_glue.o
    echo "done";
fi

# Requires: folder setup
# Creates: obj/native_app_glue.o


# Requires: obj/native_app_glue.o
# Creates: obj/libnative_app_glue.a

echo "gcc object file"
${CC} -c $PROJECT.c -o obj/$PROJECT.o $INCLUDE_PATHS $CFLAGS --sysroot=$ANDROID_TOOLCHAIN/sysroot
echo "gcc object file done"

# Requires: project.c
# Creates: obj/project.o

echo "gcc shared lib"
if [ "$ARCH" == "ARM64" ]; then
    ${CC} -o lib/arm64-v8a/lib$PROJECT.so obj/$PROJECT.o -shared $INCLUDE_PATHS $LDFLAGS $LDLIBS
fi
if [ "$ARCH" == "ARM" ]; then
    ${CC} -o lib/armeabi-v7a/lib$PROJECT.so obj/$PROJECT.o -shared $INCLUDE_PATHS $LDFLAGS $LDLIBS
fi
echo "gcc shared lib done"

# Requires: obj/project.o
# Creates: lib/armeabi-v7a/libproject.so


cat << EOM > AndroidManifest.xml
<?xml version='1.0' encoding="utf-8" ?>
<manifest 
	xmlns:android="http://schemas.android.com/apk/res/android" 
	package='com.seth.$PROJECT' 
	android:versionCode='1' 
    android:versionName='1.0' >
    <uses-sdk 
		android:minSdkVersion="16"/>
    <uses-feature android:glEsVersion="0x00020000" android:required="true" />
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
    <application 
        android:allowBackup="false" 
        android:label="Game">
        <activity 
            android:name="com.seth.$PROJECT.NativeLoader"
			android:theme="@android:style/Theme.NoTitleBar.Fullscreen" 
			android:configChanges="orientation|keyboardHidden|screenSize" 
            android:screenOrientation="landscape"
            android:launchMode="singleTask" 
            android:clearTaskOnLaunch="true">
            <meta-data android:name="android.app.lib_name" android:value="$PROJECT"/>
            <intent-filter>
                <action android:name='android.intent.action.MAIN'/>
                <category android:name='android.intent.category.LAUNCHER'/>
            </intent-filter>
        </activity>
    </application>
</manifest>
EOM

cat << EOM > res/values/strings.xml
<?xml version="1.0" encoding="utf-8"?> 
<resources>
    <string name="app_name">My App</string>
</resources>
EOM

cat << EOM > src/com/seth/$PROJECT/NativeLoader.java
package com.seth.$PROJECT; 
public class NativeLoader extends android.app.NativeActivity { 
    static {
        System.loadLibrary("$PROJECT"); 
    } 
} 
EOM

echo "aapt"
aapt package -f -m -S res -J src -M AndroidManifest.xml -I ${ANDROID_SDK_ROOT}/platforms/$PLATFORM/android.jar
echo "aapt done"

# Requires: AndroidManifest.xml, res/
# Creates: src/com/seth/project/R.java

echo "javac"
javac -verbose -source 1.7 -target 1.7 -d obj -bootclasspath `/usr/libexec/java_home`/jre/lib/rt.jar -classpath ${ANDROID_SDK_ROOT}/platforms/$PLATFORM/android.jar:obj -sourcepath src src/com/seth/$PROJECT/R.java src/com/seth/$PROJECT/NativeLoader.java
echo "javac done"

# Requires: src/com/seth/project/R.java, src/com/seth/project/NativeLoader.java
# Creates: obj/com/seth/project/NativeLoader.class ... R&attr.class R$string.class R.class 

echo "dx"
dx --verbose --dex --output=dex/classes.dex obj

# Requires: obj/com/seth/project/NativeLoader.class ... R&attr.class R$string.class R.class 
# Creates: dex/classes.dex

echo "aapt"

aapt package -f -M AndroidManifest.xml -S res -A assets -I ${ANDROID_SDK_ROOT}/platforms/$PLATFORM/android.jar -F apk/project.unsigned.apk dex

# Creates: project.unsigned.apk
# Note: The dot at the end is the directory the classes.dex file is in!

echo "aapt"
aapt add apk/project.unsigned.apk lib/armeabi-v7a/lib$PROJECT.so

# Does: Adds shared library to apk

echo "jarsigner"
jarsigner -keystore project.keystore -storepass whatever -keypass mypass -signedjar apk/project.signed.apk apk/project.unsigned.apk projectKey

# Does: Signs

zipalign -f 4 apk/project.signed.apk apk/project.apk

# Does: Aligns
adb install -r apk/project.apk

# Does: install
