1.  The document index (Doc.idx) keeps information about each document.
It is a fixed width ISAM (Index sequential access mode) index, orderd by docID.
The information stored in each entry includes a pointer into the repository,
a document length, a document checksum.
  The url index (url.idx) is used to convert URLs into docIDs.
It is a list of URL checksums with their corresponding docIDs and is sorted by
checksum. In order to find the docID of a particular URL, the URL's checksum
is computed and a binary search is performed on the checksums file to find its
docID.

	./DocIndex
		got Doc.idx, Url.idx, DocId2Url.idx

2.  sort Url.idx|uniq > Url.idx.sort_uniq

3. Segment document to terms, (with finding document according to the url)
	./DocSegment Tianwang.raw.2559638448
		got Tianwang.raw.2559638448.seg

4. Create forward index (docic-->termid)
	./CrtForwardIdx Tianwang.raw.2559638448.seg > moon.fidx

5.# set | grep "LANG"
LANG=en; export LANG;
sort moon.fidx > moon.fidx.sort

6. Create inverted index (termid-->docid)
	./CrtInvertedIdx moon.fidx.sort > sun.iidx


------------------------
provding service

at http://162.105.80.60/TSE/

TSESearch	CGI program for query
Snapshot	CGI program for page snapshot

