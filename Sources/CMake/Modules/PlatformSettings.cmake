
#compiller flags
if( NOT DISABLE_DEBUG )
    set( CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -D__DAVAENGINE_DEBUG__" )

endif  ()

if     ( ANDROID )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wno-invalid-offsetof" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mfloat-abi=softfp -mfpu=neon -Wno-invalid-offsetof -frtti" )    
    set( CMAKE_ECLIPSE_MAKE_ARGUMENTS -j8 )
    
elseif ( IOS     ) 
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fvisibility=hidden" )
    set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS} -O0" )
    set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} -O2" )
    
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY iPhone/iPad )
    set( CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 7.0 )

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_OSX_ARCHITECTURES armv7 armv7s i386 arm64 )

    if( NOT IOS_BUNDLE_IDENTIFIER )
        set( IOS_BUNDLE_IDENTIFIER com.davaconsulting.${PROJECT_NAME} )
        
    endif()
    
    # Fix try_compile
    set( MACOSX_BUNDLE_GUI_IDENTIFIER  ${IOS_BUNDLE_IDENTIFIER} )
    set( CMAKE_MACOSX_BUNDLE YES )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" )

elseif ( MACOS )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES )

elseif ( WIN32)
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP /EHsc" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP /EHsc" ) 
    set ( CMAKE_EXE_LINKER_FLAGS_RELEASE "/ENTRY:mainCRTStartup" )

    # undef macros min and max defined in windows.h
    add_definitions ( -DNOMINMAX )
endif  ()


##

if( WARNINGS_AS_ERRORS )

    set(LOCAL_DISABLED_WARNINGS "-Weverything \
-Werror \
-Wno-c++98-compat-pedantic \
-Wno-newline-eof \
-Wno-gnu-anonymous-struct \
-Wno-nested-anon-types \
-Wno-float-equal \
-Wno-extra-semi \
-Wno-unused-parameter \
-Wno-shadow \
-Wno-exit-time-destructors \
-Wno-documentation \
-Wno-global-constructors \
-Wno-padded \
-Wno-weak-vtables \
-Wno-variadic-macros \
-Wno-deprecated-register \
-Wno-sign-conversion \
-Wno-sign-compare \
-Wno-format-nonliteral \
-Wno-cast-align \
-Wno-conversion \
-Wno-unreachable-code \
-Wno-zero-length-array \
-Wno-switch-enum \
-Wno-c99-extensions \
-Wno-missing-prototypes \
-Wno-missing-field-initializers \
-Wno-conditional-uninitialized \
-Wno-covered-switch-default \
-Wno-deprecated \
-Wno-unused-macros \
-Wno-disabled-macro-expansion \
-Wno-undef \
-Wno-non-virtual-dtor \
-Wno-char-subscripts \
-Wno-unneeded-internal-declaration \
-Wno-unused-variable \
-Wno-used-but-marked-unused \
-Wno-missing-variable-declarations \
-Wno-gnu-statement-expression \
-Wno-missing-braces \
-Wno-reorder \
-Wno-implicit-fallthrough \
-Wno-ignored-qualifiers \
-Wno-shift-sign-overflow \
-Wno-mismatched-tags \
-Wno-missing-noreturn \
-Wno-consumed \
-Wno-sometimes-uninitialized \
-Wno-delete-non-virtual-dtor \
-Wno-header-hygiene")

    if( ANDROID )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCAL_DISABLED_WARNINGS}" ) # warnings as errors
    elseif( APPLE )
        set( LOCAL_DISABLED_WARNINGS "${LOCAL_DISABLED_WARNINGS} \
-Wno-old-style-cast \
-Wno-cstring-format-directive \
-Wno-duplicate-enum \
-Wno-unreachable-code-break \
-Wno-unreachable-code-return \
-Wno-infinite-recursion \
-Wno-objc-interface-ivars \
-Wno-direct-ivar-access \
-Wno-objc-missing-property-synthesis \
-Wno-over-aligned \
-Wno-unused-exception-parameter \
-Wno-idiomatic-parentheses \
-Wno-vla-extension \
-Wno-vla \
-Wno-overriding-method-mismatch \
-Wno-method-signatures" )

        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCAL_DISABLED_WARNINGS}" ) # warnings as errors
    elseif( WIN32 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX" )
    endif()

endif()


##
if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
  
elseif ( MACOS )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

elseif ( WIN32)
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()
