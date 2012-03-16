set GSOAP=C:\Users\David\programacion\gsoap-2.8\gsoap
set WSDL2H=%GSOAP%\bin\win32\wsdl2h
set SOAPCPP2=%GSOAP%\bin\win32\soapcpp2
set INCLUDEGSOAP=%GSOAP%\import

%SOAPCPP2% -penv -d..\..\proxy env.h

%WSDL2H% -nJasperServer -NJasperServer -y -g -I%INCLUDEGSOAP%/ -o JasperServer.h JasperServer.wsdl 

%SOAPCPP2% -C -j -d..\..\proxy -I%INCLUDEGSOAP% -n -pjasperserver -qJasperServer JasperServer.h