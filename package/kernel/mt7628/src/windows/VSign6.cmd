@echo off
echo VSign6 2013/05/24
set CURRENDIR=%cd%

for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a)
echo.
for /d %%i in (*.*) do ( cd %%i
    echo %%i\
    for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a
    )
    echo.
    for /d %%j in (*.*) do (cd %%j
        echo %%i\%%j\
        for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a
        )
        echo.
        for /d %%k in (*.*) do (cd %%k
            echo %%i\%%j\%%k\
            for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a
            )
            echo.
            for /d %%l in (*.*) do (cd %%l
                echo %%i\%%j\%%k\%%l\
                for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a
                )
                echo.
            	for /d %%m in (*.*) do (cd %%m
                	echo %%i\%%j\%%k\%%l\%%m\
                	for %%a in (*.sys *.cat) do ( call VerifySign2.cmd %%a
                	)
                	cd ..
		)
                cd ..
            )
            cd ..
        )
        cd ..
    )
    cd ..
)

cd  %CURRENDIR%
set CURRENDIR=
@echo on