#!/bin/sh

WSDL2H=../../../../gsoap-2.7.14/gsoap/bin/linux386/wsdl2h
SOAPCPP2=../../../../gsoap-2.7.14/gsoap/bin/linux386/soapcpp2
INCLUDEGSOAP=../../../../gsoap-2.7.14/gsoap/import

${WSDL2H} -nJasperServer -NJasperServer -y -g -I${INCLUDEGSOAP}/ -o JasperServer.h JasperServer.wsdl 

${SOAPCPP2} -C -d../proxy -I${INCLUDEGSOAP}/ -n -pjasperserver -qJasperServer JasperServer.h