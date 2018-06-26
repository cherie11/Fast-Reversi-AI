# Modified from John Fish's Othello Program


#Library import
from tkinter import *
from math import *
from time import *
from random import *
from copy import deepcopy
import numpy as np
import mcts


#Variable setup
nodes = 0
# depth = 4
moves = 0

#Tkinter setup
root = Tk()
screen = Canvas(root, width=500, height=600, background="#87CEFA",highlightthickness=0)
screen.pack()
total_time=0
FULL_MASK = 0xFFFFFFFFFFFFFFFF
LSB_TABLE = [0] * 64
BIT = [1 << n for n in range(64)]		
bitmap = 1
LSB_HASH = 0x07EDD5E59A4E28C2 
class Board:
	def __init__(self,human):
		self.MCTS=mcts.MCTS()
		#White goes first (0 is white and player,1 is black and computer)
		self.human=human
		self.player = 1
		self.passed = False
		self.won = False
		#Initializing an empty board
		self.array = np.zeros([8,8],dtype=np.int)
		#Initializing center values
		self.array[3][3] = 1
		self.array[3][4]=-1	
		self.array[4][3]=-1
		self.array[4][4]=1
		self.oldarray = self.array
		global BIT
		global LSB_TABLE
		global bitmap

		for i in range(64):
			LSB_TABLE[(((bitmap & (~bitmap + 1)) * LSB_HASH) & FULL_MASK) >> 58] = i
			bitmap <<= 1
		#Initializing old values
		
	#Updating the board to the screen
	def update(self):
		global total_time
		screen.delete("highlight")
		screen.delete("tile")
		for x in range(8):
			for y in range(8):
				#Could replace the circles with images later, if I want
				#TODO
				if self.oldarray[x][y]==1:
					screen.create_oval(54+50*x,54+50*y,96+50*x,96+50*y,tags="tile {0}-{1}".format(x,y),fill="#aaa",outline="#aaa")
					screen.create_oval(54+50*x,52+50*y,96+50*x,94+50*y,tags="tile {0}-{1}".format(x,y),fill="#fff",outline="#fff")

				elif self.oldarray[x][y]==-1:
					screen.create_oval(54+50*x,54+50*y,96+50*x,96+50*y,tags="tile {0}-{1}".format(x,y),fill="#000",outline="#000")
					screen.create_oval(54+50*x,52+50*y,96+50*x,94+50*y,tags="tile {0}-{1}".format(x,y),fill="#111",outline="#111")
		#Animation of new tiles
		screen.update()
		for x in range(8):
			for y in range(8):
				#Could replace the circles with images later, if I want
				if self.array[x][y]!=self.oldarray[x][y] and self.array[x][y]==1:
					screen.delete("{0}-{1}".format(x,y))
					screen.create_oval(54+50*x,54+50*y,96+50*x,96+50*y,tags="tile",fill="#aaa",outline="#aaa")
					screen.create_oval(54+50*x,52+50*y,96+50*x,94+50*y,tags="tile",fill="#fff",outline="#fff")
					screen.update()

				elif self.array[x][y]!=self.oldarray[x][y] and self.array[x][y]==-1:
					screen.delete("{0}-{1}".format(x,y))
					screen.create_oval(54+50*x,54+50*y,96+50*x,96+50*y,tags="tile",fill="#000",outline="#000")
					screen.create_oval(54+50*x,52+50*y,96+50*x,94+50*y,tags="tile",fill="#111",outline="#111")
					screen.update()

		#Drawing of highlight circles
		
		(W,B) = to_bitboard(self.array)
		legal_moves = {}
		if not self.won:
			if self.player==1 and self.human == 1:
				legal_moves = gen_movelist(B,W)
			elif self.player == 0 and self.human == 0:
				legal_moves = gen_movelist(W,B)
			for mv in legal_moves:
				x,y = to_move(mv)
				screen.create_oval(68+50*x,68+50*y,32+50*(x+1),32+50*(y+1),tags="highlight",fill="#008000",outline="#008000")
			
		self.drawScoreBoard()
		if not self.won:
			#Draw the scoreboard and update the screen
			screen.update()
			#If the computer is AI, make a move
			if self.player== 1-self.human:
			
				if self.human==0:
					cur_color=-1
				else:
					cur_color=1

				startTime = time()
				# print(self.array .ravel().tolist())
				p = self.MCTS.get_move(self.array .ravel().tolist(),cur_color,1,0,0)
				x=p.x
				y=p.y
				print('AI move:',y,x)
				deltaTime = round((time()-startTime)*100)/100
				total_time+= deltaTime
				# print('time',deltaTime)
				#Move and update screen
				self.oldarray = self.array
				self.oldarray[x][y]=cur_color
				self.array = move(self.array,x,y)
				
				self.passTest()
				
				#Switch Player
	
				
				screen.delete("time")
				screen.create_text(200,525,anchor="w", tags="time",font=("Consolas", 20),fill="white",text='TimeCost:'+str(deltaTime)+'s')
				screen.create_text(200,550,anchor="w", tags="time",font=("Consolas", 20),fill="white",text='TotalTime:'+str( round(total_time,2))+'s')
				# print(self.player)
				# print("Time for AI: ",deltaTime)	
		if self.won:
			screen.create_text(250,460,anchor="c",font=("Consolas",40),fill="red", text="The game is done!")
			screen.update()
	#Only for Human move,not AI
	#Moves to position
	
	def boardMove(self,x,y):
		#Move and update screen
		if self.human==self.player:
			if self.human==0:
				val=1
			else:
				val=-1
		else:
			if self.human==0:
				val=-1
			else:
				val=1
		# print('Turn for human:')
		# print(x,y)
		# print('player:',self.player)
		self.oldarray = self.array
		self.oldarray[x][y]=val
		self.array = move(self.array,x,y)
		self.passTest()
		# print(np.array(self.array).T)
		
		#Check if ai must pass
		# print("sub",self.won)
		#self.update()	

	
	#METHOD: Draws scoreboard to screen
	def drawScoreBoard(self):
		global moves
		#Deleting prior score elements
		screen.delete("score")

		#Scoring based on number of tiles
		player_score = 0
		computer_score = 0
		player_score=int(np.sum(self.array[self.array==1]))
		computer_score=int(-np.sum(self.array[self.array==-1]))

		if self.player==self.human:
			player_colour = "green"
			computer_colour = "gray"
		else:
			player_colour = "gray"
			computer_colour = "green"

		screen.create_oval(5,540,25,560,fill=player_colour,outline=player_colour)
		screen.create_oval(380,540,400,560,fill=computer_colour,outline=computer_colour)

		# Pushing text to screen
		screen.create_text(30,550,anchor="w", tags="score",font=("Consolas", 50),fill="white",text=player_score)
		screen.create_text(400,550,anchor="w", tags="score",font=("Consolas", 50),fill="black",text=computer_score)

		moves = player_score+computer_score


	#METHOD: Whether need to switch the player 
	def passTest(self):
		mustPass = True
		self.player = 1-self.player
		for x in range(8):
			for y in range(8):
				if valid(self.array,self.player,x,y):
					mustPass=False

		if mustPass is False:
			self.passed = False
		else:
			self.passed = True
			mustPass = True
			self.player = 1-self.player
			for x in range(8):
				for y in range(8):
					if valid(self.array,self.player,x,y):
						mustPass=False

			if mustPass==True:  
				self.won = True
				
		self.update()



#FUNCTION: Returns a board after making a move according to Othello rules
#Assumes the move is valid
def move(passedArray,x,y):
	#Must copy the passedArray so we don't alter the original
	array = deepcopy(passedArray)
	#Set colour and set the moved location to be that colour
	if board.player==0:
		colour = 1
		
	else:
		colour=-1
	array[x][y]=colour
	
	#Determining the neighbours to the square
	neighbours = []
	for i in range(max(0,x-1),min(x+2,8)):
		for j in range(max(0,y-1),min(y+2,8)):
			if array[i][j]!=0:
				neighbours.append([i,j])
	
	#Which tiles to convert
	convert = []

	#For all the generated neighbours, determine if they form a line
	#If a line is formed, we will add it to the convert array
	for neighbour in neighbours:
		neighX = neighbour[0]
		neighY = neighbour[1]
		#Check if the neighbour is of a different colour - it must be to form a line
		if array[neighX][neighY]== -colour:
			#The path of each individual line
			path = []
			
			#Determining direction to move
			deltaX = neighX-x
			deltaY = neighY-y

			tempX = neighX
			tempY = neighY

			#While we are in the bounds of the board
			while 0<=tempX<=7 and 0<=tempY<=7:
				path.append([tempX,tempY])
				value = array[tempX][tempY]
				#If we reach a blank tile, we're done and there's no line
				if value==0:
					break
				#If we reach a tile of the player's colour, a line is formed
				if value==colour:
					#Append all of our path nodes to the convert array
					for node in path:
						convert.append(node)
					break
				#Move the tile
				tempX+=deltaX
				tempY+=deltaY
				
	#Convert all the appropriate tiles
	for node in convert:
		array[node[0]][node[1]]=colour

	return array

#Method for drawing the gridlines
def drawGridBackground(outline=False):
	#If we want an outline on the board then draw one
	if outline:
		screen.create_rectangle(50,50,450,450,outline="#111")

	#Drawing the intermediate lines
	for i in range(7):
		lineShift = 50+50*(i+1)

		#Horizontal line
		screen.create_line(50,lineShift,450,lineShift,fill="#111")

		#Vertical line
		screen.create_line(lineShift,50,lineShift,450,fill="#111")

	screen.update()


def to_move(bitmove):
	return (bitmove % 8, bitmove // 8)


#Checks if a move is valid for a given array.
def valid(array,player,x,y):
	#Sets player colour
	if player==0:
		colour=1
	else:
		colour=-1
		
	#If there's already a piece there, it's an invalid move
	if array[x][y]!= 0:
		return False

	else:
		#Generating the list of neighbours
		neighbour = False
		neighbours = []
		for i in range(max(0,x-1),min(x+2,8)):
			for j in range(max(0,y-1),min(y+2,8)):
				if array[i][j]!=0:
					neighbour=True
					neighbours.append([i,j])
		#If there's no neighbours, it's an invalid move
		if not neighbour:
			return False
		else:
			#Iterating through neighbours to determine if at least one line is formed
			valid = False
			for neighbour in neighbours:

				neighX = neighbour[0]
				neighY = neighbour[1]
				
				#If the neighbour colour is equal to your colour, it doesn't form a line
				#Go onto the next neighbour
				if array[neighX][neighY]==colour:
					continue
				else:
					#Determine the direction of the line
					deltaX = neighX-x
					deltaY = neighY-y
					tempX = neighX
					tempY = neighY

					while 0<=tempX<=7 and 0<=tempY<=7:
						#If an empty space, no line is formed
						if array[tempX][tempY]==0:
							break
						#If it reaches a piece of the player's colour, it forms a line
						if array[tempX][tempY]==colour:
							valid=True
							break
						#Move the index according to the direction of the line
						tempX+=deltaX
						tempY+=deltaY
			return valid

#When the user clicks, if it's a valid move, make the move
def clickHandle(event):
	global depth
	xMouse = event.x
	yMouse = event.y
	if running:
		if xMouse>=450 and yMouse<=50:
			root.destroy()
		elif xMouse<=50 and yMouse<=50:
			screen.delete(ALL)
			runGame()
		else:
			#Is it the player's turn?
			if board.player==board.human:
				#Delete the highlights
				x = int((event.x-50)/50)
				y = int((event.y-50)/50)
				#Determine the grid index for where the mouse was clicked
				
				#If the click is inside the bounds and the move is valid, move to that location
				if 0<=x<=7 and 0<=y<=7:
					if valid(board.array,board.player,x,y):
						board.boardMove(x,y)
	else:
		#Difficulty clicking
		if 300<=yMouse<=350:
			
			#One star
			if 25<=xMouse<=155:

				playGame(human=1)
			'''
			#Two star
			elif 180<=xMouse<=310:
				depth = 4
				playGame()
			'''
			#Three star
			if 335<=xMouse<=465:
				playGame(human=0)

def keyHandle(event):
	symbol = event.keysym
	if symbol.lower()=="r":
		playGame()
	elif symbol.lower()=="q":
		root.destroy()

def create_buttons():
		#Restart button
		#Background/shadow
		screen.create_rectangle(0,5,50,55,fill="#000033", outline="#000033")
		screen.create_rectangle(0,0,50,50,fill="#000088", outline="#000088")

		#Arrow
		screen.create_arc(5,5,45,45,fill="#000088", width="2",style="arc",outline="white",extent=300)
		screen.create_polygon(33,38,36,45,40,39,fill="white",outline="white")

		#Quit button
		#Background/shadow
		screen.create_rectangle(450,5,500,55,fill="#330000", outline="#330000")
		screen.create_rectangle(450,0,500,50,fill="#880000", outline="#880000")
		#"X"
		screen.create_line(455,5,495,45,fill="white",width="3")
		screen.create_line(495,5,455,45,fill="white",width="3")
	
def runGame():
	global running
	running = False
	#Title and shadow
	screen.create_text(250,203,anchor="c",text="Reversi",font=("Comic Sans MS", 50),fill="#aaa")
	screen.create_text(250,200,anchor="c",text="Reversi",font=("Comic Sans MS", 50),fill="#fff")
	screen.create_text(250,453,anchor="c",text="AI agent with MCTS",font=("Comic Sans MS", 30),fill="#aaa")
	screen.create_text(250,450,anchor="c",text="AI agent with MCTS",font=("Comic Sans MS", 30),fill="#fff")
	
	#Creating the difficulty buttons

	for i in range(3):
		#Background
		if i !=1:
			round_rectangle(20+155*i, 310, 155+155*i, 355, fill="#4169E1", outline="#4169E1")
			round_rectangle(25+155*i, 305, 155+155*i, 350, fill="#1E90FF", outline="#1E90FF")
		else:
			screen.create_text(240,310,anchor="c",text="\u2605", font=("Comic Sans MS",50),fill="#b29600")
			screen.create_text(240,311,anchor="c",text="\u2605", font=("Comic Sans MS", 50),fill="#b29600")
			screen.create_text(240,312,anchor="c",text="\u2605", font=("Comic Sans MS", 50),fill="#ffd700")
			
		spacing = 130/(i+2)

		if i ==0:
			screen.create_text(25+1*spacing+155*i,326,anchor="c",text="Human Black", font=("Comic Sans MS", 15),fill="#ffd700")
		elif i==2:
			screen.create_text(25+2*spacing+155*i,325,anchor="c",text="Human White", font=("Comic Sans MS", 15),fill="#ffd700")
		

	screen.update()


def round_rectangle(x1, y1, x2, y2, r=25, **kwargs):    
    points = (x1+r, y1, x1+r, y1, x2-r, y1, x2-r, y1, x2, y1, x2, y1+r, x2, y1+r, x2, y2-r, x2, y2-r, x2, y2, x2-r, y2, x2-r, y2, x1+r, y2, x1+r, y2, x1, y2, x1, y2-r, x1, y2-r, x1, y1+r, x1, y1+r, x1, y1)
    return screen.create_polygon(points, **kwargs)

def playGame(human):
	global board, running
	running = True
	screen.delete(ALL)
	if human!=0:
		string="Human is Black"
	else:
		string="Human is White"
	screen.create_text(250,30,anchor="c",text=string,font=("Comic Sans MS", 40,"bold"),fill="#4169E1")
	create_buttons()
	board = 0

	#Draw the background
	drawGridBackground()

	#Create the board and update it
	board = Board(human)
	board.update()

def gen_movelist(P,O):
    mlist=[]
    legal_moves=move_gen(P,O)
    leagl_binary=legal_moves
    while leagl_binary != 0:
        mv, leagl_binary = pop_lsb(leagl_binary)
        mlist.append(mv)
    return mlist

def move_gen(P, O):
    mask = O & 0x7E7E7E7E7E7E7E7E
    return ((move_gen_sub(P, mask, 1) \
            | move_gen_sub(P, O, 8)  \
            | move_gen_sub(P, mask, 7) \
            | move_gen_sub(P, mask, 9)) & ~(P|O))

def move_gen_sub(P, mask, dir):
    dir2 = dir * 2
    flip1  = mask & (P << dir)
    flip2  = mask & (P >> dir)
    flip1 |= mask & (flip1 << dir)
    flip2 |= mask & (flip2 >> dir)
    mask1  = mask & (mask << dir)
    mask2  = mask & (mask >> dir)
    flip1 |= mask1 & (flip1 << dir2)
    flip2 |= mask2 & (flip2 >> dir2)
    flip1 |= mask1 & (flip1 << dir2)
    flip2 |= mask2 & (flip2 >> dir2)
    return (flip1 << dir) | (flip2 >> dir)

def to_bitboard(board):
    W = 0
    B = 0
    for r in range(8):
        for c in range(8):
            if board[c][r] == -1:
                B |= BIT[8 * r + c]
            elif board[c][r] == 1:
                W |= BIT[8 * r + c]
    return (W, B)

def pop_lsb(bitmap):
    #get least bit
	l = LSB_TABLE[(((bitmap & (~bitmap + 1)) * LSB_HASH) & FULL_MASK) >> 58]
	bitmap &= bitmap - 1
	return l, bitmap & FULL_MASK

runGame()

#Binding, setting
screen.bind("<Button-1>", clickHandle)
screen.bind("<Key>",keyHandle)
screen.focus_set()

#Run forever
root.wm_title("Reversi")
root.mainloop()