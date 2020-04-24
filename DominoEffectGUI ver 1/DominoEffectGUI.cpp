#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <string>

using namespace std;

const double PI = 3.141592653589793238463;

// moveType Guide:
//  F - Drop Domino

//	W - Both Wheels Forward
//	I - Both Wheels Backward

//  D - Left Wheel Forward 
//  A - Right Wheel Forward

//  J - Left Wheel Backward
//  L - Right Wheel Backward

struct pathNode {
	char moveType; // Indicates movement type for path action
	int numDominoes; // For forward/backward movement, set to -1 for other movement types
	double turnDegrees; // Desired degrees to turn for one-wheel movement, set to -1 for forward/backward
	pathNode *nextNode;
	pathNode *lastNode;
};

// Note that wheel radius and distances are in
// inches, while degrees are in... degrees
class GUIcontroller {
	private:
		double wheel_radius; // Radius of wheels driven by motors
		double wheel_distance; // Distance between the two wheels controlling movement
		double degrees_per_step; // Degrees the motors move for each step
		double distance_between_dominoes; // Distance between dominoes

		double distance_per_step; // Distance traveled by each step of a motor
		double turn_circumference; // Circumference of a circle with a radius that is the distance between the two wheels
		double step_degree_change; // Degree change each step causes when traveling along the circumference
		double degrees_between_dominoes; // degrees between each domino around the circumference of the circle

		int total_actions; // Total actions set by the user

		pathNode *head;
		pathNode *tail;

	public:
		GUIcontroller(double r = 1.5, double w = 10, double d = 1.8, double dd = 1.2) {
			wheel_radius = r;
			wheel_distance = w;
			degrees_per_step = d;
			distance_between_dominoes = dd;

			distance_per_step = 2*PI*wheel_radius * degrees_per_step / 360.0;
			turn_circumference = 2*PI*wheel_distance;
			step_degree_change = distance_per_step / turn_circumference * 360.0;
			degrees_between_dominoes = distance_between_dominoes / turn_circumference * 360.0;

			total_actions = 0;

			head = NULL;
			tail = NULL;
		}

		// Returns number of path actions currently input
		int get_num_actions() {
			return total_actions;
		}

		// Adds desired path action
		void add_path_action(char moveType, int numDominoes, double turnDegrees) {
			pathNode *newNode = new pathNode();
			newNode->moveType = moveType;
			newNode->nextNode = NULL;

			if (moveType == 'W' || moveType == 'I') {
				newNode->numDominoes = numDominoes;
				newNode->turnDegrees = -1.0;
			}
			else {
				newNode->numDominoes = -1;
				newNode->turnDegrees = turnDegrees;
			}

			if (head == NULL) {
				newNode->lastNode = NULL;
				head = newNode;
				tail = newNode;
			}
			else {
				newNode->lastNode = tail;
				tail->nextNode = newNode;
				tail = tail->nextNode;
			}

			total_actions++;
		}

		// Removes last path action
		void remove_last() {
			pathNode *removeNode = tail;

			tail = tail->lastNode;
			tail->nextNode = NULL;
			
			delete removeNode;

			total_actions--;
		}

		// Outputs path instructions to .txt
		void finalize_path() {
			ofstream my_file;
			my_file.open("domino_effect_gui_output.txt");
			
			pathNode* curNode = head;

			while (curNode != NULL) {
				// Prints instructions for a straight line of dominoes
				if (curNode->moveType == 'W' || curNode->moveType == 'I') {
					for (int j = 0; j < curNode->numDominoes; j++) {
						my_file << 'F';

						for (double i = distance_per_step; i < distance_between_dominoes; i += distance_per_step) {
							my_file << curNode->moveType;
						}
					}
				}

				// Prints instructions for curve made up of dominoes
				else {
					// Input # dominoes
					if (curNode->turnDegrees <= 0) {
						for (int j = 0; j < curNode->numDominoes; j++) {
							my_file << 'F';

							for (double i = distance_per_step; i < distance_between_dominoes; i += distance_per_step) {
								my_file << 'W';
								my_file << curNode->moveType;
							}
						}
					}
					// Input degrees
					else {
						double d = 0.0;
						double distance_thresh = 0.0;

						while (d / 2 / turn_circumference * 360.0 < curNode->turnDegrees) {
							if (d >= distance_thresh) {
								my_file << 'F';
								distance_thresh += distance_between_dominoes;
							}

							my_file << 'W';
							my_file << curNode->moveType;
							d += 2 * distance_per_step;
						}

						while (d <= distance_thresh) {
							my_file << 'W';
							d += distance_per_step;
						}
					}

					curNode = curNode->nextNode;
				}
			}

			my_file.close();
		}

		// Deletes all path actions
		void clear_path() {
			while (head != NULL) {
				pathNode *removeNode = head;
				head = head->nextNode;
				delete removeNode;
			}

			tail = NULL;
			total_actions = 0;
		}

		// Loads input values from config.txt
		void load_config(string file_name) {
			clear_path();

			ifstream in_file;
			in_file.open(file_name);

			string line;

			while (!in_file.eof()) {
				in_file >> line;
				string sub_str = line.substr(line.find("=") + 1);

				//cout << line << endl;
				//cout << sub_str << endl << endl;

				if (line.find("wheel_radius") != -1) {
					wheel_radius = stod(sub_str);
				}
				else if (line.find("wheel_distance") != -1) {
					wheel_distance = stod(sub_str);
				}
				else if (line.find("degrees_per_step") != -1) {
					degrees_per_step = stod(sub_str);
				}
				else if (line.find("distance_between_dominoes") != -1) {
					distance_between_dominoes = stod(sub_str);
				}

				distance_per_step = 2 * PI*wheel_radius * degrees_per_step / 360.0;
				turn_circumference = 2 * PI*wheel_distance;
				step_degree_change = distance_per_step / turn_circumference * 360.0;
				degrees_between_dominoes = distance_between_dominoes / turn_circumference * 360.0;
			}

			in_file.close();
		}
};

int main(int argc, char *argv[]) {
	GUIcontroller myGUI = GUIcontroller();
	
	string file_name = "config.txt";

	myGUI.load_config(file_name);

	char input = '_';

	while (input != 'e' && input != 'E') {
		cout << "w - Add Action, Straight Line" << endl;
		cout << "A - Add Action, Turn Left (# dominoes input)" << endl;
		cout << "D - Add Action, Turn Right (# dominoes input)" << endl << endl;

		cout << "1 - Add Action, Turn Left (degrees input)" << endl;
		cout << "2 - Add Action, Turn Right (degrees input)" << endl << endl;

		cout << "R - remove last action" << endl;
		cout << "C - Clear Path and Start Over" << endl;
		cout << "F - Finalize Path & Print to .txt" << endl << endl;

		cout << "E - Exit Program" << endl;
		cout << "" << endl << endl;

		cout << "You are on Path Action #" << myGUI.get_num_actions()+1 << endl << endl;

		cout << "Input Desired Action: ";
		cin >> input;
		cout << endl;



		if (input == 'w' || input == 'W') {
			int num_dominoes;

			cout << "Straight Line Selected" << endl;
			cout << "Enter # of Dominoes to Set Down: ";
			cin >> num_dominoes;
			cout << endl;

			myGUI.add_path_action('W', num_dominoes, -1);
		}

		else if (input == 'a' || input == 'A') {
			int num_dominoes;

			cout << "Turn Left Selected" << endl;
			cout << "Enter # of Dominoes to Set Down: ";
			cin >> num_dominoes;
			cout << endl;

			myGUI.add_path_action('A', num_dominoes, -1);
		}

		else if (input == 'd' || input == 'D') {
			int num_dominoes;

			cout << "Turn Right Selected" << endl;
			cout << "Enter # of Dominoes to Set Down: ";
			cin >> num_dominoes;
			cout << endl;

			myGUI.add_path_action('D', num_dominoes, -1);
		}

		else if (input == '1') {
			double turn_degrees;

			cout << "Turn Left Selected" << endl;
			cout << "Enter Degrees to Turn: ";
			cin >> turn_degrees;
			cout << endl;

			myGUI.add_path_action('A', -1, turn_degrees);
		}

		else if (input == '2') {
			double turn_degrees;

			cout << "Turn Right Selected" << endl;
			cout << "Enter Degrees to Turn: ";
			cin >> turn_degrees;
			cout << endl;

			myGUI.add_path_action('D', -1, turn_degrees);
		}

		else if (input == 'r' || input == 'R') {
			myGUI.remove_last();
		}

		else if (input == 'c' || input == 'C') {
			myGUI.clear_path();
		}

		else if (input == 'f' || input == 'F') {
			myGUI.finalize_path();
		}

		else if(input != 'e' && input != 'E') {
			cout << "WARNING: INVALID INPUT. NO ACTION TAKEN" << endl;
		}

		cout << endl << endl << endl;
	}

	return 0;
}