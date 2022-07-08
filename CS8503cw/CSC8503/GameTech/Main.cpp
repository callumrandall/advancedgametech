 #include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "..//CSC8503Common/PushdownState.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "..//CSC8503Common/PushdownMachine.h"

#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/

//void TestStateMachine() {
//	StateMachine * testMachine = new StateMachine();
//	int data = 0;
//	State* A = new State([&](float dt)->void
//		{
//		std::cout << "I’m in state A!\n";
//		data++;
//		}
//	);
//	
//	State * B = new State([&](float dt)->void
//		{
//			std::cout << "I’m in state B!\n";
//			data--;
//		}
//	);
//	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
//		{
//			return data > 10;
//		}
//	);
//	StateTransition * stateBA = new StateTransition(B, A, [&](void)->bool
//		{
//			return data < 0;
//		}
//	);
//
//	testMachine->AddState(A);
//	testMachine->AddState(B);
//	testMachine->AddTransition(stateAB);
//	testMachine->AddTransition(stateBA);
//	
//	for (int i = 0; i < 100; ++i) {
//		testMachine->Update(1.0f);
//	}
//}

//class PauseScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	}
//	void OnAwake() override {
//		std::cout << " Press U to unpause game !\n";
//	}
//};
//
//class LevelOne : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		LoadLevel();
//		return PushdownResult::NoChange;
//	};
//	void OnAwake() override {
//		std::cout << "Starting game!\n";
//	}
//protected:
//};
//
//class IntroScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		Debug::FlushRenderables(dt);
//		renderer->Render();
//		renderer->DrawString("1 for Level 1", Vector2(30, 30), Vector4(0, 0, 0, 1));
//		renderer->DrawString("2 for Level 2", Vector2(30, 40), Vector4(0, 0, 0, 1));
//
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
//			*newState = new LevelOne();
//			return PushdownResult::Push;
//		}
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	};
//	void OnAwake() override {
//		std::cout << "Welcome to a really awesome game !\n";
//		std::cout << "Press Space To Begin or escape to quit !\n";
//	}
//protected:
//	GameWorld* world;
//	GameTechRenderer* renderer;
//};
//
//void TestPushdownAutomata(Window* w) {
//	PushdownMachine machine(new IntroScreen());
//	while (w->UpdateWindow()) {
//		float dt = w->GetTimer()->GetTimeDeltaSeconds();
//		if (!machine.Update(dt)) {
//			return;
//		}
//	}
//}

vector <Vector3> testNodes;
Vector3 startPos(80, 0, 20);
Vector3 endPos(160, 0, 160);
void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");
	NavigationPath outPath;
	testNodes.clear();
	
	//Vector3 startPos(160, 0, 20);
	//Vector3 endPos(160, 0, 160);

	
	bool found = grid.FindPath(startPos, endPos, outPath);
	
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}
void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];
		
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}


int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	//TestPushdownAutomata(w);

	if (!w->HasInitialised()) {
		return -1;
	}	
	srand(time(0));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	TestPathfinding();

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		if (!g->world1) {
			startPos = g->enemyPos();
			endPos = g->playerPos();
			TestPathfinding();
			if (testNodes.size() > 1 && testNodes.size() < 8) {
				//g->moveEnemy(testNodes[0], testNodes[1]);
				g->chase = true;
			}
			else {
				g->chase = false;
			}

			if (g->chase) {
				g->chaseStart = testNodes[0];
				g->chaseEnd = testNodes[1];
			}
			if (!g->chase) {
				startPos = g->enemyPos();
				endPos = Vector3(100, 5, 130);
				TestPathfinding();
				if (testNodes.size() > 1) {
					g->chaseStart = testNodes[0];
					g->chaseEnd = testNodes[1];
				}
			}
			//DisplayPathfinding();
		}

		//g->playerPos();
		//g->enemyPos();
		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}

//void LoadLevel() {
//	Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
//
//	/*if (!w->HasInitialised()) {
//		return -1;
//	}*/
//	srand(time(0));
//	w->ShowOSPointer(false);
//	w->LockMouseToWindow(true);
//
//	TutorialGame* g = new TutorialGame();
//	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
//	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
//		float dt = w->GetTimer()->GetTimeDeltaSeconds();
//		if (dt > 0.1f) {
//			std::cout << "Skipping large time delta" << std::endl;
//			continue; //must have hit a breakpoint or something to have a 1 second frame time!
//		}
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
//			w->ShowConsole(true);
//		}
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
//			w->ShowConsole(false);
//		}
//
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
//			w->SetWindowPosition(0, 0);
//		}
//
//		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
//
//		g->UpdateGame(dt);
//	}
//	Window::DestroyGameWindow();
//}