# API Webserver

### Project info
Simple api server that store wav files and can return info based on queries. 
Server is being run at localhost:8080

### Example post
curl -X POST -H "X-File-Name: sample-5s.wav" -H "Content-Type: audio/wav" --data-binary @Downloads/sample-3s.wav http://localhost:8080/files

### Queries
* /info?name=<string>
  * returns all info on file with that name
* /download?name=<string> 
  * returns file with that name
* /list
  * returns list of file names based on combination of below queries
    * ?name=<string>
    * ?minduration=<int>
    * ?maxduration=<int>
    * ?duration=<int>
    * ?minsize=<int>
    * ?maxsize=<int>
    * ?size=<int>