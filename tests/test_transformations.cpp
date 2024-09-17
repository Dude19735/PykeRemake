#define BOOST_TEST_MODULE TestTransformations
#include "boost/test/included/unit_test.hpp"

#include <iostream>
#include <vector>

#include "utilities.hpp"
#include "../src/Vk_Rotations.hpp"

BOOST_AUTO_TEST_SUITE(TestTransformations)

BOOST_AUTO_TEST_CASE(GLM_test_simple_rotation_1) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(1, 0, 0));
	points.push_back(glm::vec3(0, 1, 0));
	points.push_back(glm::vec3(0, 0, 1));

	glm::mat3 matrix(points[0], points[1], points[2]);

	UT::Transformation::qVecRot(points, glm::pi<float_t>() / 2, glm::vec3(0, 0, 1));
	UT::Transformation::qMatRot(matrix, glm::pi<float_t>() / 2, glm::vec3(0, 0, 1));

	std::vector<glm::vec3> expected;
	expected.push_back(glm::vec3(0, 1, 0));
	expected.push_back(glm::vec3(-1, 0, 0));
	expected.push_back(glm::vec3(0, 0, 1));

	BOOST_CHECK(UT::TestUtilities::assert_compare(points, expected) == true);
	BOOST_CHECK(UT::TestUtilities::assert_compare(
		std::vector<glm::vec3> {
			glm::column(matrix, 0), 
			glm::column(matrix, 1), 
			glm::column(matrix, 2)},
			expected) == true
	);
	//std::cout << "hello world" << std::endl;
}

BOOST_AUTO_TEST_CASE(GLM_test_simple_rotation_2) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(1, 0, 0));
	points.push_back(glm::vec3(0, 1, 0));
	points.push_back(glm::vec3(0, 0, 1));

	glm::mat3 matrix(points[0], points[1], points[2]);

	UT::Transformation::qVecRot(points, glm::pi<float_t>() / 2, glm::vec3(0, 1, 0));
	UT::Transformation::qMatRot(matrix, glm::pi<float_t>() / 2, glm::vec3(0, 1, 0));

	std::vector<glm::vec3> expected;
	expected.push_back(glm::vec3(0, 0, -1));
	expected.push_back(glm::vec3(0, 1, 0));
	expected.push_back(glm::vec3(1, 0, 0));

	BOOST_CHECK(UT::TestUtilities::assert_compare(points, expected) == true);
	BOOST_CHECK(UT::TestUtilities::assert_compare(
		std::vector<glm::vec3> {
			glm::column(matrix, 0), 
			glm::column(matrix, 1), 
			glm::column(matrix, 2)}, 
			expected) == true
	);
}

BOOST_AUTO_TEST_CASE(GLM_test_simple_rotation_3) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(1, 0, 0));
	points.push_back(glm::vec3(0, 1, 0));
	points.push_back(glm::vec3(0, 0, 1));

	glm::mat3 matrix(points[0], points[1], points[2]);

	UT::Transformation::qVecRot(points, glm::pi<float_t>() / 2, glm::vec3(1, 0, 0));
	UT::Transformation::qMatRot(matrix, glm::pi<float_t>() / 2, glm::vec3(1, 0, 0));

	std::vector<glm::vec3> expected;
	expected.push_back(glm::vec3(1, 0, 0));
	expected.push_back(glm::vec3(0, 0, 1));
	expected.push_back(glm::vec3(0, -1, 0));

	BOOST_CHECK(UT::TestUtilities::assert_compare(points, expected) == true);
	BOOST_CHECK(UT::TestUtilities::assert_compare(
		std::vector<glm::vec3> {
			glm::column(matrix, 0), 
			glm::column(matrix, 1), 
			glm::column(matrix, 2)}, 
			expected) == true
	);
}

BOOST_AUTO_TEST_CASE(GLM_test_simple_rotation_4) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(1, 0, 0));
	points.push_back(glm::vec3(0, 1, 0));
	points.push_back(glm::vec3(0, 0, 1));

	glm::mat3 matrix(points[0], points[1], points[2]);

	UT::Transformation::qVecRot(points, glm::pi<float_t>() / 4, glm::vec3(0, 0, 1));
	UT::Transformation::qMatRot(matrix, glm::pi<float_t>() / 4, glm::vec3(0, 0, 1));

	std::vector<glm::vec3> expected;
	expected.push_back(glm::vec3(0.707106781187, 0.707106781187, 0));
	expected.push_back(glm::vec3(-0.707106781187, 0.707106781187, 0));
	expected.push_back(glm::vec3(0, 0, 1));

	BOOST_CHECK(UT::TestUtilities::assert_compare(points, expected) == true);
	BOOST_CHECK(UT::TestUtilities::assert_compare(
		std::vector<glm::vec3> {
			glm::column(matrix, 0), 
			glm::column(matrix, 1), 
			glm::column(matrix, 2)}, 
			expected) == true
	);
}

BOOST_AUTO_TEST_CASE(GLM_test_simple_rotation_5) {
	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(1, 0, 0));
	points.push_back(glm::vec3(0, 1, 0));
	points.push_back(glm::vec3(0, 0, 1));

	glm::mat3 matrix(points[0], points[1], points[2]);

	UT::Transformation::qVecRot(points, 2 * glm::pi<float_t>(), glm::vec3(1, 1, 1));
	UT::Transformation::qMatRot(matrix, 2 * glm::pi<float_t>(), glm::vec3(1, 1, 1));

	std::vector<glm::vec3> expected;
	expected.push_back(glm::vec3(1, 0, 0));
	expected.push_back(glm::vec3(0, 1, 0));
	expected.push_back(glm::vec3(0, 0, 1));

	BOOST_CHECK(UT::TestUtilities::assert_compare(points, expected) == true);
	BOOST_CHECK(UT::TestUtilities::assert_compare(
		std::vector<glm::vec3> {
			glm::column(matrix, 0), 
			glm::column(matrix, 1), 
			glm::column(matrix, 2)}, 
			expected) == true
	);
}

BOOST_AUTO_TEST_CASE(GLM_test_translation) {
	float data[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
	};

	glm::mat4 m = glm::make_mat4(data);

	glm::vec4 v = glm::vec4(1, 2, 3, 1);

	std::cout << glm::to_string(m) << std::endl << glm::to_string(v) << std::endl;

	m = glm::translate(m, glm::vec3(1, 2, 3));

	std::cout << glm::to_string(m) << std::endl << glm::to_string(m * v) << std::endl;

}

BOOST_AUTO_TEST_SUITE_END()
