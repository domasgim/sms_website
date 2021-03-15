<?php

use Denpa\ZeroMQ\Connection;
use React\ZMQ\SocketWrapper;

class ConnectionTest extends TestCase
{
    /**
     * Test magic calls to context through connection.
     *
     * @return void
     */
    public function testMagicCall()
    {
        $socket = zeromq()->connection()->getSocket(ZMQ::SOCKET_PUSH);

        $this->assertInstanceOf(SocketWrapper::class, $socket);
    }

    /**
     * Test publish action.
     *
     * @return void
     */
    public function testPublish()
    {
        $loop = $this->mockLoop();
        $socket = $this->mockSocket();
        $context = $this->mockContext();

        $socket
            ->expects($this->once())
            ->method('bind');

        $socket
            ->expects($this->once())
            ->method('sendmulti')
            ->with(['test', json_encode(['foo' => 'bar'])]);

        $socket
            ->expects($this->once())
            ->method('end');

        $context
            ->expects($this->once())
            ->method('__call')
            ->with('getSocket', [ZMQ::SOCKET_PUB])
            ->willReturn($socket);

        $loop
            ->expects($this->once())
            ->method('run');

        zeromq()
            ->setLoop($loop)
            ->setContext($context)
            ->publish(['test'], ['foo' => 'bar'])
            ->run();
    }

    /**
     * Test pull action.
     *
     * @return void
     */
    public function testPull()
    {
        $loop = $this->mockLoop();
        $socket = $this->mockSocket();
        $context = $this->mockContext();
        $callback = $this->mockCallable([
            $this->callback(function ($message) {
                return $message == 'test';
            }),
        ], $this->exactly(2), $this->returnValue(false));

        $socket
            ->expects($this->once())
            ->method('bind');

        $socket
            ->expects($this->exactly(2))
            ->method('on')
            ->withConsecutive(
                ['messages'],
                ['message']
            )
            ->will($this->returnCallback(function ($event, $callback) {
                $callback('test');
            }));

        $socket
            ->expects($this->once())
            ->method('close')
            ->will($this->returnCallback(function () use ($socket) {
                $socket->closed = true;
            }));

        $context
            ->expects($this->once())
            ->method('__call')
            ->with('getSocket', [ZMQ::SOCKET_PULL])
            ->willReturn($socket);

        $loop
            ->expects($this->once())
            ->method('run');

        zeromq()
            ->setLoop($loop)
            ->setContext($context)
            ->pull($callback)
            ->run();
    }

    /**
     * Test push action.
     *
     * @return void
     */
    public function testPush()
    {
        $loop = $this->mockLoop();
        $socket = $this->mockSocket();
        $context = $this->mockContext();

        $socket
            ->expects($this->once())
            ->method('connect')
            ->will($this->returnSelf());

        $socket
            ->method('send')
            ->with('test');

        $socket
            ->expects($this->once())
            ->method('end');

        $context
            ->expects($this->once())
            ->method('__call')
            ->with('getSocket', [ZMQ::SOCKET_PUSH])
            ->willReturn($socket);

        $loop
            ->expects($this->once())
            ->method('run');

        zeromq()
            ->setLoop($loop)
            ->setContext($context)
            ->push('test')
            ->run();
    }

    /**
     * Test subscribe action.
     *
     * @return void
     */
    public function testSubscribe()
    {
        $loop = $this->mockLoop();
        $socket = $this->mockSocket();
        $context = $this->mockContext();
        $callback = $this->mockCallable([
            $this->callback(function ($message) {
                return $message == 'test';
            }),
        ], $this->exactly(2), $this->returnValue(false));

        $context
            ->expects($this->once())
            ->method('__call')
            ->with('getSocket', [ZMQ::SOCKET_SUB])
            ->willReturn($socket);

        $socket
            ->expects($this->once())
            ->method('connect');

        $socket
            ->expects($this->exactly(2))
            ->method('subscribe')
            ->withConsecutive(
                ['foo'],
                ['bar']
            );

        $socket
            ->expects($this->exactly(2))
            ->method('on')
            ->withConsecutive(
                ['messages'],
                ['message']
            )
            ->will($this->returnCallback(function ($event, $callback) {
                $callback('test');
            }));

        $socket
            ->expects($this->once())
            ->method('close')
            ->will($this->returnCallback(function () use ($socket) {
                $socket->closed = true;
            }));

        $loop
            ->expects($this->once())
            ->method('run');

        zeromq()
            ->setLoop($loop)
            ->setContext($context)
            ->subscribe(['foo', 'bar'], $callback)
            ->run();
    }
}
