#! /bin/sh
# file: examples/equality_test.sh

. ./config.inc

config

temp=temp
result=${temp}/result
result2=${temp}/result2

oneTimeSetUp()
{
  mkdir ${temp}
  touch ${result}
  touch ${result2}
}

oneTimeTearDown()
{
  rm ${result}
  rm ${result2}
  rmdir ${temp}
}

setUp()
{
  var2=2
}

testEquality()
{
  assertEquals ${var2} 2
}

nottestPartyLikeItIs1999()
{
  year=`date '+%Y'`
  assertEquals "[123] It's not 1999 :-(" \
      '1999' "${year}"
  #echo "ae: ${_ASSERT_EQUALS_}"
  #${_ASSERT_EQUALS_} 'not equal' 1 2
}

testLineNo()
{
  # this assert will have line numbers included (e.g. "ASSERT:[123] ...")
  #echo "ae: ${_ASSERT_EQUALS_}"  ${_ASSERT_EQUALS_} 'not equal' 1 2

  # this assert will not have line numbers included (e.g. "ASSERT: ...")
  assertEquals 'not equal ae' '1' '1'
  #echo ${_ASSERT_EQUALS_}
}

printError()
{
  ret=$?
  #echo "print $?"
  if [ $# == 1 ]; then
    if [ ${ret} != '0' -a -f $1 ] ; then
      echo -e "${Red}Error${NC}"
	  cat $1
    fi
  fi
  unset ret
}

testCsUIMode()
{
  cs Camera.Menu.UIMode '' > ${result} 2>/dev/null
  assertEquals 'Camera.Menu.UIMode' 'OK' "$(grep OK ${result})"
  printError {$result}
  
  cs UIMode '' > ${result} 2>/dev/null
  assertEquals 'UIMode' 'OK' "$(grep OK ${result})"
  printError {$result}
}

testCsWildcard()
{
  cs "Camera.Menu.*" '' > ${result} 2>/dev/null
  assertEquals 'Get wildcard' 'OK' "$(grep OK ${result})"
  printError {$result}
  ret1=$(grep -c 'Camera.Menu.' ${result})
  assertTrue 'Get wildcard number' "[ ${ret1} > 10 ]"
  printError {$result}

  cs "Camera.Menu.*" '' > ${result} 2>/dev/null
  assertEquals 'Get wildcard 2' 'OK' "$(grep OK ${result})"
  printError {$result}
  ret2=$(grep -c 'Camera.Menu.' ${result})
  assertTrue 'Get wildcard number 2' "[ ${ret1} == ${ret2} ]"
  printError {$result}
}

testCsH264res()
{
  cs Camera.Preview.H264.w '' > ${result} 2>/dev/null
  assertEquals 'Get Reslution' 'OK' "$(grep OK ${result})"
  w=$(grep "Camera.Preview.H264.w=" ${result} | sed " s/[[:alnum:]]*[\.\=]//g")
  assertTrue "Check width(${w}) fail" "[ $w == 640 -o $w == 1280 ]"
  printError {$result}
  unset w

  cs Camera.Preview.H264.h '' > ${result} 2>/dev/null
  h=$(grep "Camera.Preview.H264.h=" ${result} | sed " s/[[:alnum:]]*[\.\=]//g")
  assertTrue "Check height(${h}) fail" "[ $h == 360 -o $h == 480 -o $h == 720 ]"
  printError {$result}
  unset h
}

testCsStatus()
{
  cs "status" '' > ${result} 2>/dev/null
  assertEquals 'Get Status' 'OK' "$(grep OK ${result})"
  #w=$(grep "Camera.Preview.H264.w=" ${result} | sed " s/[[:alnum:]]*[\.\=]//g")
  #assertTrue "Check width(${w}) fail" "[ $w == 640 -o $w == 1280 ]"
  cat ${result}
  echo ""
  printError {$result}
}

testBrca()
{
  /bin/nc -4lu 49142 > ${result} &
  cs broadcast > ${result2} 2>/dev/null
  assertEquals 'Get Broadcast' 'OK' "$(grep OK ${result2})"
  printError "${result2}"
  cat ${result}
  echo ""
  assertEquals 'Broadcast Message' "IP=${IP}" "$(grep 'IP=' ${result})"
  printError ${result}
}

suite()
{
	suite_addTest testCsUIMode
	suite_addTest testCsWildcard
	suite_addTest testCsUIMode
	suite_addTest testCsWildcard
	#suite_addTest testCsH264res
	suite_addTest testBrca
	suite_addTest testCsStatus
}

#typeset -F | awk '{print $3}' | grep ^test 
#echo -e "\n"

# load shunit2
. ${shunit2}/src/shell/shunit2
#. 2.1/src/shell/shunit2
