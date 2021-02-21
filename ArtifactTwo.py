#!/usr/bin/python
import json
from bson import json_util
import bottle
from bottle import route, run, request, abort, response, post, get, put, delete
from pymongo import MongoClient
import pprint

# Set target db & collection
connection = MongoClient('localhost', 27017)
db = connection['city']
collection = db['inspections']


# PyMongo CRUD Methods            
# Inserts a new document into Collection.

def insert_document(document):
  try:
    collection.save(document)
  except TypeError as te:
    abor(400, str(te))
  else:
    result = collection.find(document)
    return result
  
  
# Searches Collection for documents matching passed criteria

def find_document(document):
  result = []

  try:
    data = collection.find(document)
    
    for document in data:
      result.append(document)
  except TypeError as te:
    abort(400, str(te))
  else:
    return result
  
  
# Updates documents based on search criteria.
# Updated documents are then returned.

def edit_document(criteria, document):
  try:
    collection.update_one(criteria,{"$set" : document})
    result = collection.find(criteria)
  except TypeError as te:
    abort(400, str(te))
  except Exception as we:
    abort(400, str(we))
  except:
    abort(400, "Bad Request")
  else:
    return result
  
  
# Searches for documents based on passed criteria.
# If documents are found, they are printed.
# Documents are then deleted.

def remove_document(document):
  try:
    collection.delete_one(document)
    result = "True"
  except TypeError as te:
    abort(400, str(te))
    
  return result


#     Set up URI paths for REST service     


# URI path for greeting user
# Accepts name query param from request
# Returns name in a greeting in JSON format

@route('/hello', method='GET')
def get_greeting():
  try:
    request.query.name
    name=request.query.name
    if name:
      string = "{hello: \""+request.query.name+"\"}\n"
  except NameError:
    abort(404, 'No parameter for id %s' % id)
    
  return json.loads(json.dumps(string,indent=4, default=json_util.default))
  
# URI path for accepting a JSON object
# Accepts JSON object and returns elements in a JSON-parsed string

@route('/strings', method='POST')
def post_strings():
  try:
    data = request.json  
  except:
    abort(404, "Bad data")
   
  if data:
    string = "{first: \""+data['string1']+"\", second: \""+data['string2']+"\"}\n"
  
  response.headers['Content-Type'] = 'application/json'
  return json.loads(json.dumps(string, indent=4, default=json_util.default))
  
# URI path for creating a new document in city db
# Accepts a JSON object and adds to city.inspections collection
#   by calling insert_document()

@post('/create')
def create_document():
  try:
    data = request.json
  except:
    abort(404, "Bad data")
    
  try:
    result = insert_document(data)
    
    response.headers['Content-Type'] = 'application/json'
    return json.dumps(list(result), indent=4, default=json_util.default)
  except NameError as ne:
    abort(400, str(ne))


# URI path for searching for a document
# Uses the business_name query parameter
#   to search MongoDb using find_document()

@get('/read')
def read_document():
  try:
    name = request.query.business_name
    cursor = find_document({"business_name" : name})
    
    if cursor:
      response.content_type = 'application/json'
      return json.dumps(list(cursor), indent=4, default=json_util.default)
    else:
      abort(404, "No documents found")
  except NameError as ne:
    abort(404, str(ne))
    
# URI path for updating an existing documents
# Uses the id query parameter to search MongoDb using edit_document(),
#   then updates the returned documents with the result query parameter

@get('/update')
def update_document():
  try:
    business_id = request.query.id
    business_result = request.query.result
    criteria = {"id" : business_id}
    change = {"result" : business_result}
    cursor = edit_document(criteria, change)
    
    if cursor:
      response.content_type = 'application/json'
      return json.dumps(list(cursor), indent=4, default=json_util.default)
    else:
      abort(404, "No documents found")
  except NameError as ne:
    abort(404, str(ne))
    
# URI path for deleting a documents
# Uses the id query parameter to search MongoDb using remove_document(),
#   matching documents are deleted

@get('/delete')
def delete_document():
  
  try:
    business_id = request.query.id
    result = remove_document({"id" : business_id})
    
    if result == "True":
      return "delete success\n"
    else:
      abort(404, "File not found")
  except:
    abort(400, "Bad Request")

# Application entry point
# Starts REST service
if __name__ == '__main__':
  #app.run(debug=True)
  run(host='localhost',port=8080, debug=True)
