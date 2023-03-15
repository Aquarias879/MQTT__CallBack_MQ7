from pymongo import MongoClient

client = MongoClient("mongodb://localhost:27017/")
db = client["db_gas"]
posts = db["key"]

recent_posts = posts.find().sort("created_at", -1).limit(10)
for post in recent_posts:
    print(post)

