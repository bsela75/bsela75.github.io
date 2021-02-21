import com.mongodb.client.MongoDatabase; 
import com.mongodb.MongoClient; 
import com.mongodb.MongoCredential;  
public class ConnectDB { 
   
   public static void main( String args[] ) {  
      
      // Creating a Mongo client 
      MongoClient mongo = new MongoClient( "localhost" , 27017 ); 
   
      // Creating Credentials 
      MongoCredential credential; 
      credential = MongoCredential.createCredential("user", "city", 
         "password".toCharArray()); 
      System.out.println("Connected to the database successfully");  
      
      // Accessing the database 
      MongoDatabase database = mongo.getDatabase("city"); 
      System.out.println("Credentials ::"+ credential);     
   } 
}

mport com.mongodb.client.MongoDatabase; 
import org.bson.Document; 
import com.mongodb.MongoClient; 
import com.mongodb.MongoCredential;  
public class selectCollection { 
   
   public static void main( String args[] ) {  
      
      // Creating a Mongo client 
      MongoClient mongo = new MongoClient( "localhost" , 27017 ); 
     
      // Creating Credentials 
      MongoCredential credential; 
      credential = MongoCredential.createCredential("user", "city", 
         "password".toCharArray()); 
      System.out.println("Connected to the database successfully");  
      
      // Accessing the database 
      MongoDatabase database = mongo.getDatabase("city");  
      
      // Retrieving a collection
      MongoCollection<Document> collection = database.getCollection("inspections"); 
      System.out.println("Collection inspections selected successfully"); 
   }
}
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import org.bson.Document;
import com.mongodb.MongoClient;
public class InsertDocument {
	public static void main( String args[] ) {
	
	// Creating a Mongo client
	MongoClient mongo = new MongoClient( "localhost" , 27017 );
	
	// Accessing the database
	MongoDatabase database = mongo.getDatabase("city");
	
	// Retrieving a collection
	MongoCollection<Document> collection = database.getCollection("inspections");
	System.out.println("Collection inspections selected successfully");

	//Inserting document into the collection
	collection.insertOne(document);
	System.out.println("Document inserted successfully");
	}
}


import com.mongodb.client.FindIterable; 
import com.mongodb.client.MongoCollection; 
import com.mongodb.client.MongoDatabase; 
import com.mongodb.client.model.Filters; 
import com.mongodb.client.model.Updates; 
import java.util.Iterator; 
import org.bson.Document;  
import com.mongodb.MongoClient; 
import com.mongodb.MongoCredential;  
public class UpdateDocuments { 
   
   public static void main( String args[] ) {  
      
      // Creating a Mongo client 
      MongoClient mongo = new MongoClient( "localhost" , 27017 ); 
     
      // Creating Credentials 
      MongoCredential credential; 
      credential = MongoCredential.createCredential("user", "city", 
         "password".toCharArray()); 
      System.out.println("Connected to the database successfully");  
      
      // Accessing the database 
       MongoDatabase database = mongo.getDatabase("city");  
      // Retrieving a collection 
      MongoCollection<Document> collection = database.getCollection("inspections");
      System.out.println("Collection inspections selected successfully"); 
      collection.updateOne(Filters.eq("title", 1), Updates.set("", ));       
      System.out.println("Document update successfully...");  
      
      // Retrieving the documents after updation 
      // Getting the iterable object
      FindIterable<Document> iterDoc = collection.find(); 
      int i = 1; 
      // Getting the iterator 
      Iterator it = iterDoc.iterator(); 
      while (it.hasNext()) {  
         System.out.println(it.next());  
         i++; 
      }     
   }  
}


import com.mongodb.client.FindIterable; 
import com.mongodb.client.MongoCollection; 
import com.mongodb.client.MongoDatabase; 
import com.mongodb.client.model.Filters;  
import java.util.Iterator; 
import org.bson.Document; 
import com.mongodb.MongoClient; 
import com.mongodb.MongoCredential;  
public class DeleteDocuments { 
   
   public static void main( String args[] ) {  
   
      // Creating a Mongo client 
      MongoClient mongo = new MongoClient( "localhost" , 27017 );
      
      // Creating Credentials 
      MongoCredential credential; 
      credential = MongoCredential.createCredential("user", "city", 
         "password".toCharArray()); 
      System.out.println("Connected to the database successfully");  
      
      // Accessing the database 
      MongoDatabase database = mongo.getDatabase("city"); 
      // Retrieving a collection
      MongoCollection<Document> collection = database.getCollection("inspections");
      System.out.println("Collection inspections selected successfully"); 
      // Deleting the documents 
      collection.deleteOne(Filters.eq("", "")); 
      System.out.println("Document deleted successfully...");  
      
      // Retrieving the documents after updation 
      // Getting the iterable object 
      FindIterable<Document> iterDoc = collection.find(); 
      int i = 1; 
      // Getting the iterator 
      Iterator it = iterDoc.iterator(); 
      while (it.hasNext()) {  
         System.out.println(it.next());  
         i++; 
      }       
   } 
}


