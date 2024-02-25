from fastapi import FastAPI
from pydantic import BaseModel

class gameChoice(BaseModel):
    number: int

app = FastAPI()

games = {
    1: "Pong",
    2: "Snake"
}

@app.get("/")
async def root():
    return {"message": "Hello World"}

@app.post("/games")
async def chooseGame(choice: gameChoice):
    return {"message": f"Starting {games[choice.number]}"}


@app.get("/home")
async def goHome():
    with open('home.html') as homepage:
        return homepage.read()