
char	*Input;
char	*Output;
char	*Temp;
char	*Key;
int		nCount;	// ���� ���� ��ȣ

// inverse operation�� ��� 
// input�� �������� offset�� output�� �����ϸ� �ǰ� 
// key�� temp�� ����ƴ� offset�� �״�� key�� temp�� �����ϸ� �ȴ�.(����� key�� Tempó�� �����Ѵ�)
start:
xor()
{
	Output[0]	= Input[0] ^ Key[0];
}

xor_1()
{
	Output[0]	= Input[1] ^ Key[0];
}
//~xor_1()
//{
//	Output[1]	= Input[0] ^ Key[0];
//}
xor_2()
{
	Output[0]	= Input[2] ^ Key[0];
}
//~xor_2()
//{
//	Output[2]	= Input[0] ^ Key[0];
//}
xor_3()
{
	Output[0]	= Input[3] ^ Key[0];
}
//~xor_3()
//{
//	Output[3]	= Input[0] ^ Key[0];
//}
xor_4()
{
	Output[0]	= Input[4] ^ Key[0];
}
//~xor_4()
//{
//	Output[4]	= Input[0] ^ Key[0];
//}
xor_5()
{
	Output[0]	= Input[5] ^ Key[0];
}
//~xor_5()
//{
//	Output[5]	= Input[0] ^ Key[0];
//}
xor_6()
{
	Output[0]	= Input[6] ^ Key[0];
}
//~xor_6()
//{
//	Output[6]	= Input[0] ^ Key[0];
//}
xor_7()
{
	Output[0]	= Input[7] ^ Key[0];
}
//~xor_7()
//{
//	Output[7]	= Input[0] ^ Key[0];
//}
// ���� ��� decoding�� ���� ���� �ڵ带 ����Ѵ�.
//~xor()
//{
//	Output[0]	= Input[0] ^ Key[0];
//}

not()
{
	Output[0]	= ~Input[0];
}

not_1()
{
	Output[0]	= ~Input[5];
}

not_2()
{
	Output[2]	= ~Input[12];
}
not_3()
{
	Output[3]	= ~Input[12];
}
not_4()
{
	Output[4]	= ~Input[12];
}
not_5()
{
	Output[5]	= ~Input[12];
}
notkey_xor()
{
	Temp[0]		= ~Key[0];
	Output[0]	= Input[0] ^ Temp[0];
}

notcount_xor()
{
	Temp[0]		= ~nCount;
	Output[0]	= Input[0] ^ Temp[0];
}

shift_left()
{
	Output[0]	= Input[0] << Key[0];
}

// ���� �ڵ尡 ����Ǵ� inverse�� shift�� ��� �Ųٷ� �����Ѵ�
//~shift_left()
//{
//	Output[0]	= Input[0] >> Key[0];
//}

notkey_shift_left()
{
	Temp[0]		= ~Key[0];
	Output[0]	= Input[0] << Temp[0];
}

shift_right()
{
	Output[0]	= Input[0] >> Key[0];
}

shift_right_1()
{
	Output[3]	= Input[5] >> Key[1];
}
//~shift_right_1()
//{
//	Output[5]	= Input[3] << Key[1];
//}
shift_right_2()
{
	Output[3]	= Input[1] >> Key[1];
}
//~shift_right_2()
//{
//	Output[1]	= Input[3] << Key[1];
//}
notkey_shift_right()
{
	Temp[0]		= ~Key[0];
	Output[0]	= Input[0] >> Temp[0];
}
value_shift_left()
{
	Output[0]	= Input[0] >> 1;
}

value_shift_right()
{
	Output[0]	= Input[0] << 1;
}

count_shift_left()
{
	Output[0]	= Input[0] << nCount;
}

count_shift_right()
{
	Output[0]	= Input[0] >> nCount;
}
end: