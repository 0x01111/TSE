rm -f WebData.*
rm -f core.*
rm -f Tianwang.raw.*
rm -f Link4SE.raw.*
rm -f *.gz
rm -f nohup.out

rm -f tse_unvisited.url
rm -f visited.all
rm -f tse_md5.visitedurl
rm -f tse_md5.visitedpage
rm -f link4SE.url
rm -f link4History.url
rm -f repli

sort tse_unreachHost.list > 11
uniq 11 > 22
cp 22 tse_unreachHost.list
rm -f 11 22

